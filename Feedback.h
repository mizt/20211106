class Feedback : public ComputeShaderBase {

    protected:

        int _frame = 0;
        std::vector<unsigned int *> _buffer;

        id<MTLBuffer> _resolution = nil;
        id<MTLBuffer> _amount  = nil;

        unsigned int *exec() {
            
            ComputeShaderBase::update();
            
            this->_frame++;
            this->copy(this->_buffer[this->_frame&1],this->_texture[2]);

            return this->_buffer[this->_frame&1];
        }
    
    public:
    
        void reset(unsigned int *src) {
            for(int i=0; i<this->_height; i++) {
                for(int j=0; j<this->_width; j++) {
                    this->_buffer[0][i*this->_width+j] = this->_buffer[1][i*this->_width+j] = src[i*this->_width+j]; // ABGR
                }
            }
        }
    
        unsigned int *exec(unsigned int *src,unsigned int *buf) {
        
            [this->_texture[0] replaceRegion:MTLRegionMake2D(0,0,this->_width,this->_height) mipmapLevel:0 withBytes:src bytesPerRow:(this->_width)<<2];
        
            [this->_texture[1] replaceRegion:MTLRegionMake2D(0,0,this->_width,this->_height) mipmapLevel:0 withBytes:buf bytesPerRow:(this->_width)<<2];
        
            return this->exec();
        }
        
        unsigned int *exec(unsigned int *buf) {
            
            this->exec(this->_buffer[this->_frame&1],buf);
            
            return this->exec();
        }
    
        unsigned int *bytes() {
            return this->_buffer[this->_frame&1];
        }
        
        Feedback(int w,int h,NSString *path) : ComputeShaderBase(w,h) {
        
            this->_buffer.push_back(new unsigned int[w*h]);
            this->_buffer.push_back(new unsigned int[w*h]);
            
            for(int i=0; i<h; i++) {
                for(int j=0; j<w; j++) {
                    this->_buffer[0][i*w+j] = this->_buffer[1][i*w+j] = 0xFF000000; // ABGR
                }
            }
            
            MTLTextureDescriptor *RGBA8Unorm = ComputeShaderBase::descriptor(MTLPixelFormatRGBA8Unorm,w,h);
            RGBA8Unorm.usage = MTLTextureUsageShaderWrite|MTLTextureUsageShaderRead;

            MTLTextureDescriptor *RG16Unorm = ComputeShaderBase::descriptor(MTLPixelFormatRG16Unorm,w,h);
            RG16Unorm.usage = MTLTextureUsageShaderWrite|MTLTextureUsageShaderRead;

            this->_texture.push_back([this->_device newTextureWithDescriptor:RGBA8Unorm]);
            this->_texture.push_back([this->_device newTextureWithDescriptor:RG16Unorm]);
            this->_texture.push_back([this->_device newTextureWithDescriptor:RGBA8Unorm]);
        
            this->_params.push_back(this->newBuffer(sizeof(float)*2));
            float *resolution = (float *)[this->_params[0] contents];
            resolution[0] = this->_width;
            resolution[1] = this->_height;
            
            this->_params.push_back(this->newBuffer(sizeof(float)*2));
            float *amount = (float *)[this->_params[1] contents];
            amount[0] = 0.05;
            
            ComputeShaderBase::setup(path);
        }
    
        ~Feedback() {
            
            for(int k=0; k<this->_texture.size(); k++) {
                this->_texture[k] = nil;
            }
            
            for(int k=0; k<this->_buffer.size(); k++) {
                delete[] this->_buffer[k];
            }
        }
};


