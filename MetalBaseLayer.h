namespace PixelFormat {
	MTLPixelFormat ARGB = MTLPixelFormatRGBA8Unorm;
	MTLPixelFormat ABGR = MTLPixelFormatBGRA8Unorm;
}

template <typename T>
class MetalBaseLayer {
	
	protected:
		
		T *_data;
		
        NSString *_name;

		CAMetalLayer *_metalLayer = nil;
        MTLRenderPassDescriptor *_renderPassDescriptor;
    
        id<MTLDevice> _device = nil;
        id<MTLCommandQueue> _commandQueue  = nil;
		
        id<CAMetalDrawable> _metalDrawable  = nil;
		
        id<MTLTexture> _drawabletexture = nil;
			
        id<MTLBuffer> _verticesBuffer = nil;
        id<MTLBuffer> _indicesBuffer = nil;
    
        id<MTLLibrary> _library;
        id<MTLRenderPipelineState> _renderPipelineState = nil;
        MTLRenderPipelineDescriptor *_renderPipelineDescriptor = nil;
		
        bool _useArgumentEncoder = false;

		id<MTLArgumentEncoder> _argumentEncoder;
			
		bool _isInit = false;
			
		int _width;
		int _height;
		CGRect _frame;

        bool _blendingEnabled = false;
		unsigned int _mode = 0;
    
        MTLPixelFormat _pixelformat = PixelFormat::ARGB;
    
        static const BOOL isEqString(NSString *a, NSString *b) {
            return [a compare:b]==NSOrderedSame;
        }
        
        static const BOOL isEqClassName(id a, NSString *b) {
#if TARGET_OS_OSX
            return [[a className] compare:b]==NSOrderedSame;
#else
            return isEqString(NSStringFromClass([a class]),b);
#endif
        }
        
		virtual void setColorAttachment(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) {
                        
			colorAttachment.blendingEnabled = YES;
			colorAttachment.rgbBlendOperation = MTLBlendOperationAdd;
			colorAttachment.alphaBlendOperation = MTLBlendOperationAdd;
			colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
			colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
			colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
			colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOne;
		}
    
        bool updateShader() {
            
            id<MTLFunction> vertexFunction = [this->_library newFunctionWithName:@"vertexShader"];
            if(!vertexFunction) return nil;
            id<MTLFunction> fragmentFunction = [this->_library newFunctionWithName:@"fragmentShader"];
            if(!fragmentFunction) return nil;
            
            if(this->_useArgumentEncoder) this->_argumentEncoder = [fragmentFunction newArgumentEncoderWithBufferIndex:0];
            
            this->_renderPipelineDescriptor.vertexFunction = vertexFunction;
            this->_renderPipelineDescriptor.fragmentFunction = fragmentFunction;
            NSError *error = nil;
            this->_renderPipelineState = [this->_device newRenderPipelineStateWithDescriptor:this->_renderPipelineDescriptor error:&error];
            if(error||!this->_renderPipelineState) return true;
            return false;
        }
        
        bool setupShader() {
            
            this->_renderPipelineDescriptor = [MTLRenderPipelineDescriptor new];
            this->_renderPipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
            this->_renderPipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
            this->_renderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            if(!this->_blendingEnabled) {
                this->_renderPipelineDescriptor.colorAttachments[0].blendingEnabled = NO;
            }
            else {
                this->setColorAttachment(this->_renderPipelineDescriptor.colorAttachments[0]);
            }
            
			return this->updateShader();
		}
		
	public:
		
		T *data() { return this->_data; }
		
        void setPixelFormat(MTLPixelFormat pixelFormat) {
            this->_pixelformat = pixelFormat;
        }
    
		MetalBaseLayer(int x=4,int y=4) {
			this->_data = new T(x,y);
		}

		~MetalBaseLayer() {
			delete this->_data;
		}
		
		virtual bool setup() {
                            
            this->_verticesBuffer = [this->_device newBufferWithBytes:this->_data->vertices length:this->_data->VERTICES_SIZE*sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
            if(!this->_verticesBuffer) return false;
            
            this->_indicesBuffer = [this->_device newBufferWithBytes:this->_data->indices length:this->_data->INDICES_SIZE*sizeof(short) options:MTLResourceOptionCPUCacheModeDefault];
            if(!this->_indicesBuffer) return false;
           
			return true;
		} 
		
		virtual id<MTLCommandBuffer> setupCommandBuffer() {
						
            id<MTLCommandBuffer> commandBuffer = [this->_commandQueue commandBuffer];
            MTLRenderPassColorAttachmentDescriptor *colorAttachment = this->_renderPassDescriptor.colorAttachments[0];
            colorAttachment.texture = this->_metalDrawable.texture;
            
            colorAttachment.loadAction  = MTLLoadActionClear;
            colorAttachment.clearColor  = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
            colorAttachment.storeAction = MTLStoreActionStore;
            
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:this->_renderPassDescriptor];
            [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
            [renderEncoder setRenderPipelineState:this->_renderPipelineState];
            [renderEncoder setVertexBuffer:this->_verticesBuffer offset:0 atIndex:0];

            [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:this->_data->INDICES_SIZE indexType:MTLIndexTypeUInt16 indexBuffer:this->_indicesBuffer indexBufferOffset:0];
                
            [renderEncoder endEncoding];
            
            [commandBuffer presentDrawable:this->_metalDrawable];
            this->_drawabletexture = this->_metalDrawable.texture;
            return commandBuffer;
            
		}
		
        virtual bool init(int width, int height, NSString *shader=@"default.metallib", NSString *identifier=nil, bool blendingEnabled=false) {
			
			this->_frame.size.width = this->_width = width;
			this->_frame.size.height = this->_height = height;
			if(this->_metalLayer==nil) this->_metalLayer = [CAMetalLayer layer];
			this->_device = MTLCreateSystemDefaultDevice();
			this->_metalLayer.device = this->_device;
			this->_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

#if TARGET_OS_OSX
			this->_metalLayer.colorspace = [[NSScreen mainScreen] colorSpace].CGColorSpace;
#else
			this->_metalLayer.colorspace = CGColorSpaceCreateDeviceRGB();
#endif
			
			this->_metalLayer.opaque = NO;
			this->_metalLayer.framebufferOnly = NO;

#if TARGET_OS_OSX
			this->_metalLayer.displaySyncEnabled = YES;
#endif
			this->_metalLayer.drawableSize = CGSizeMake(this->_width,this->_height);
			this->_commandQueue = [this->_device newCommandQueue];
			if(!this->_commandQueue) return false;
			NSError *error = nil;
                
            this->_name = [shader stringByDeletingPathExtension];
            
            if(FileManager::extension(shader,@"metallib")) {
                
                id<MTLLibrary> lib= [this->_device newLibraryWithFile:FileManager::path(shader,identifier) error:&error];
                
                if(lib&&error==nil) {
                    this->_library = lib;
                }
                else {
                    return false;
                }
            }
            else if(FileManager::extension(shader,@"json")) {
                
                bool err = true;
                
                NSString *path = FileManager::path(shader,identifier);
                NSString *json= [[NSString alloc] initWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&error];
                if(error==nil) {
                    NSData *jsonData = [json dataUsingEncoding:NSUnicodeStringEncoding];
                    NSMutableDictionary *dict = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers error:&error];
                    if(error==nil) {
                        if(dict[@"metallib"]&&(isEqClassName(dict[@"metallib"],@"NSTaggedPointerString")||isEqClassName(dict[@"metallib"],@"__NSCFString"))) {
                            
                            NSString *path;
                            if(identifier==nil) {
                                path = [[NSBundle mainBundle] bundlePath];
                            }
                            else {
                                NSBundle *bundle = [NSBundle bundleWithIdentifier:identifier];
                                path = [bundle resourcePath];
                            }
                            
                            id<MTLLibrary> lib= [this->_device newLibraryWithFile:[NSString stringWithFormat:@"%@/%@",path,dict[@"metallib"]] error:&error];
                             if(lib&&error==nil) {
                                this->_library = lib;
                                err = false;
                             }
                             else {
                                return false;
                             }
                            
                        }
                    }
                }
                
                if(err) return false;
                
            }
            else {
                NSLog(@"%@",[shader pathExtension]);
                return false;
            }
			
			this->_blendingEnabled = blendingEnabled;
			if(this->setupShader()) return false;	
			this->_isInit = this->setup();
			return this->_isInit;
		}
		
		bool isInit() {
			return this->_isInit;
		}
    
		id<MTLTexture> drawableTexture() { 
			return this->_drawabletexture; 
		}
		
		void cleanup() { 
			this->_metalDrawable = nil;
            this->_renderPassDescriptor = nil;
		}
		
		bool reloadShader(dispatch_data_t data) {
			NSError *error = nil;
			this->_library = [this->_device newLibraryWithData:data error:&error];
			if(error||!this->_library) return true;
			if(this->updateShader()) return true;
			return false;
		}
		
		id<MTLCommandBuffer> prepareCommandBuffer() {
			if(!this->_metalDrawable) {
				this->_metalDrawable = [this->_metalLayer nextDrawable];
			}
			if(!this->_metalDrawable) {
                this->_renderPassDescriptor = nil;
			}
			else {
                
                this->_renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

			}
			if(this->_metalDrawable&&this->_renderPassDescriptor) {
				return this->setupCommandBuffer();
			}
			return nil;
		}
		
		void update(void (^onComplete)(id<MTLCommandBuffer>)) {
			
			if(this->_isInit==false) return; 
						
            id<MTLCommandBuffer> commandBuffer = this->prepareCommandBuffer();
            if(commandBuffer) {
                [commandBuffer addCompletedHandler:onComplete];
                [commandBuffer commit];
                [commandBuffer waitUntilCompleted];
            }
		}
		
		CAMetalLayer *layer() {
			return this->_metalLayer;
		}
};
