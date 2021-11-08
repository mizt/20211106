class Mask : public ComputeShaderBase {

    protected:

        int _frame = 0;
    
        unsigned int *exec() {
            
            ComputeShaderBase::update();
            
            this->_frame++;
            this->copy(this->_buffer[this->_frame&1],this->_texture[2]);
        
            return this->_buffer[this->_frame&1];
        }
        
    public:
    
        void exec(unsigned int *brushStroke, unsigned int *triangles) {
            
            this->replace(this->_texture[0],brushStroke);
            this->replace(this->_texture[1],triangles);
            
            this->exec();
        }
    
        unsigned int *bytes() {
            return this->_buffer[this->_frame&1];
        }
            
        Mask(int w,int h,NSString *path) : ComputeShaderBase(w,h) {
        
            this->_buffer.push_back(new unsigned int[w*h]);
            this->_buffer.push_back(new unsigned int[w*h]);

            for(int i=0; i<h; i++) {
                for(int j=0; j<w; j++) {
                    this->_buffer[0][i*w+j] = this->_buffer[1][i*w+j] = 0xFF000000; // ABGR
                }
            }
            
            MTLTextureDescriptor *descriptor = ComputeShaderBase::descriptor(MTLPixelFormatRGBA8Unorm,w,h);
            descriptor.usage = MTLTextureUsageShaderWrite|MTLTextureUsageShaderRead;

            for(int k=0; k<3; k++) {
                this->_texture.push_back([this->_device newTextureWithDescriptor:descriptor]);
            }

            ComputeShaderBase::setup(path);
        }
    
        ~Mask() {
        }
    
};
