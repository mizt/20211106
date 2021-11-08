#import "MetalBaseLayer.h"

template <typename T>
class MetalLayer : public MetalBaseLayer<T> {
	
	private:
		
		id<MTLTexture> _texture;
		id<MTLBuffer> _texcoordBuffer;
    
		id<MTLBuffer> _argumentEncoderBuffer;
		
	public:
		
		id<MTLTexture> texture() { 
			return this->_texture; 
		}
		
		bool setup() {
			
			MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:this->_width height:this->_height mipmapped:NO];
			if(!texDesc) return false;
			
			this->_texture = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_texture) return false;
			
			if(MetalBaseLayer<T>::setup()==false) return false;						

			this->_texcoordBuffer = [this->_device newBufferWithBytes:this->_data->texcoord length:this->_data->TEXCOORD_SIZE*sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_texcoordBuffer) return false;
			
            this->_argumentEncoderBuffer = [this->_device newBufferWithLength:sizeof(float)*[this->_argumentEncoder encodedLength] options:MTLResourceOptionCPUCacheModeDefault];

            [this->_argumentEncoder setArgumentBuffer:this->_argumentEncoderBuffer offset:0];
            [this->_argumentEncoder setTexture:this->_texture atIndex:0];
			
			return true;
		} 
		
		id<MTLCommandBuffer> setupCommandBuffer() {
						
			id<MTLCommandBuffer> commandBuffer = [this->_commandQueue commandBuffer];
			MTLRenderPassColorAttachmentDescriptor *colorAttachment = this->_renderPassDescriptor.colorAttachments[0];
			colorAttachment.texture = this->_metalDrawable.texture;
			colorAttachment.loadAction = MTLLoadActionClear;
			colorAttachment.clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,0.0f);
			colorAttachment.storeAction = MTLStoreActionStore;
			
			id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:this->_renderPassDescriptor];
			[renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
			[renderEncoder setRenderPipelineState:this->_renderPipelineState];
			[renderEncoder setVertexBuffer:this->_verticesBuffer offset:0 atIndex:0];
			[renderEncoder setVertexBuffer:this->_texcoordBuffer offset:0 atIndex:1];
			
			[renderEncoder useResource:this->_texture usage:MTLResourceUsageSample];
			[renderEncoder setFragmentBuffer:this->_argumentEncoderBuffer offset:0 atIndex:0];
			
			[renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:this->_data->INDICES_SIZE indexType:MTLIndexTypeUInt16 indexBuffer:this->_indicesBuffer indexBufferOffset:0];
			
			[renderEncoder endEncoding];
			[commandBuffer presentDrawable:this->_metalDrawable];
			this->_drawabletexture = this->_metalDrawable.texture;
			return commandBuffer;
		}
		
		MetalLayer(CAMetalLayer *layer=nil, int x=0, int y=0) : MetalBaseLayer<T>(x,y) {
            if(layer) {
                this->_metalLayer = layer;
            }
            this->_useArgumentEncoder = true;
		}
		
		~MetalLayer() {}
};
