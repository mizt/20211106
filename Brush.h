class Brush : public ComputeShaderBase {

    public:
    
        unsigned int *exec() {
            ComputeShaderBase::update();
            this->copy(this->_buffer[0],this->_texture[0]);
            return this->_buffer[0];
        }
        
        unsigned int *bytes() {
            return this->_buffer[0];
        }
        
        Brush(int w,int h,NSString *path) : ComputeShaderBase(w,h) {
        
            this->_buffer.push_back(new unsigned int[w*h]);
            this->fill(this->_buffer[0],0x0);
            
            MTLTextureDescriptor *RGBA8Unorm = ComputeShaderBase::descriptor(MTLPixelFormatRGBA8Unorm,w,h);
            RGBA8Unorm.usage = MTLTextureUsageShaderWrite|MTLTextureUsageShaderRead;

            this->_texture.push_back([this->_device newTextureWithDescriptor:RGBA8Unorm]);
            
            this->_params.push_back(this->newBuffer(sizeof(float)*2));
            float *resolution = (float *)[this->_params[0] contents];
            resolution[0] = w;
            resolution[1] = h;
            
            ComputeShaderBase::setup(path);
        }
    
        ~Brush() {
        }
};

