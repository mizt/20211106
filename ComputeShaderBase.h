class ComputeShaderBase {
    
    private:
        
        bool _init = false;
    
    protected:

        id<MTLDevice> _device = MTLCreateSystemDefaultDevice();
        id<MTLFunction> _function;
        id<MTLComputePipelineState> _pipelineState;
        id<MTLLibrary> _library;
        std::vector<id<MTLTexture>> _texture;
        std::vector<id<MTLBuffer>> _params;
        std::vector<unsigned int *> _buffer;

        int _width = 0;
        int _height = 0;

        ComputeShaderBase(int w, int h) {
            this->_width = w;
            this->_height = h;
        }
        
        ~ComputeShaderBase() {
            
            this->_device = nil;
            this->_function = nil;
            this->_pipelineState = nil;
            this->_library = nil;
            
            for(int k=0; k<this->_params.size(); k++) {
                this->_params[k] = nil;
            }
 
            for(int k=0; k<this->_texture.size(); k++) {
                this->_texture[k] = nil;
            }
            
            for(int k=0; k<this->_buffer.size(); k++) {
                delete[] this->_buffer[k];
            }
        }
    
        MTLTextureDescriptor *descriptor(MTLPixelFormat format,int w,int h) {
            return [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:w height:h mipmapped:NO];
        }
    
        void replace(id<MTLTexture> texture, unsigned int *data) {
           [texture replaceRegion:MTLRegionMake2D(0,0,this->_width,this->_height) mipmapLevel:0 withBytes:data bytesPerRow:(this->_width)<<2];
        }
 
        void copy(unsigned int *data, id<MTLTexture> texture) {
            [texture getBytes:data bytesPerRow:this->_width<<2 fromRegion:MTLRegionMake2D(0,0,this->_width,this->_height) mipmapLevel:0];
        }
    
        void fill(unsigned int *data,unsigned int abgr) {
            for(int i=0; i<this->_height; i++) {
                for(int j=0; j<this->_width; j++) {
                    this->_buffer[0][i*this->_width+j] = abgr;
                }
            }
        }
    
        id<MTLBuffer> newBuffer(long length, MTLResourceOptions options = MTLResourceOptionCPUCacheModeDefault) {
            return [this->_device newBufferWithLength:length options:options];
        }
        
        void setup(NSString *filename, NSString *identifier=nil, NSString *func=@"processimage") {
            
            NSError *error = nil;
            NSString *path = FileManager::path(filename,identifier);
            
            this->_library = [this->_device newLibraryWithFile:path  error:&error];

            if(this->_library) {
                this->_function = [this->_library newFunctionWithName:func];
                this->_pipelineState = [this->_device newComputePipelineStateWithFunction:this->_function error:&error];
                
                if(error==nil) this->_init = true;
            }
        }
    
        void update() {
            
            if(this->_init) {
            
                id<MTLCommandQueue> queue = [this->_device newCommandQueue];

                id<MTLCommandBuffer> commandBuffer = queue.commandBuffer;
                id<MTLComputeCommandEncoder> encoder = commandBuffer.computeCommandEncoder;
                [encoder setComputePipelineState:this->_pipelineState];
                
                for(int k=0; k<this->_texture.size(); k++) {
                    [encoder setTexture:this->_texture[k] atIndex:k];
                }
                                      
                for(int k=0; k<this->_params.size(); k++) {
                    [encoder setBuffer:this->_params[k] offset:0 atIndex:k];
                }
                
                int w = (int)this->_texture[0].width;
                int h = (int)this->_texture[0].height;
                
                int tx = 1;
                int ty = 1;
                
                for(int k=1; k<5; k++) {
                    if(w%(1<<k)==0) tx = 1<<k;
                    if(h%(1<<k)==0) ty = 1<<k;
                }
                
                MTLSize threadGroupSize = MTLSizeMake(tx,ty,1);
                MTLSize threadGroups = MTLSizeMake(w/tx,h/ty,1);
                
                [encoder dispatchThreadgroups:threadGroups threadsPerThreadgroup:threadGroupSize];
                [encoder endEncoding];
                [commandBuffer commit];
                [commandBuffer waitUntilCompleted];
            }
        }
};
