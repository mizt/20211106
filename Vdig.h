#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
namespace stb_image {
    #import "stb_image.h"
}

class Vdig {
    
    private:
        
        const int WIDTH = 1080;
        const int HEIGHT = 1920;
    
        id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate;
        
        AVCaptureDevice *device;
        NSDictionary *settings;
        AVCaptureDeviceInput *deviceInput;
        AVCaptureVideoDataOutput *dataOutput;
        AVCaptureConnection *videoConnection;
        AVCaptureSession *session;
        
    public:
        
        unsigned int *buffer;
        
        Vdig() {
            
            //NSLog(@"Vdig");
            

#if TARGET_OS_IOS && !TARGET_OS_SIMULATOR
            
            this->buffer = new unsigned int[WIDTH*HEIGHT];
            for(int k=0; k<WIDTH*HEIGHT; k++) this->buffer[k] = 0;

            if(objc_getClass("Delegate")==nil) { objc_registerClassPair(objc_allocateClassPair(objc_getClass("NSObject"),"Delegate",0)); }
            Class Delegate = objc_getClass("Delegate");
            addMethod(Delegate,@"captureOutput:didOutputSampleBuffer:fromConnection:",^(id me,AVCaptureOutput *output,CMSampleBufferRef sampleBuffer,AVCaptureConnection *connection) {
                
                CVImageBufferRef buf = CMSampleBufferGetImageBuffer(sampleBuffer);
                CVPixelBufferLockBaseAddress(buf,0);
                                
                int width = (int)CVPixelBufferGetWidth(buf);
                int height = (int)CVPixelBufferGetHeight(buf);
                
                if(width==WIDTH&&height==HEIGHT) {
                    
                    int row = ((int)CVPixelBufferGetBytesPerRow(buf))>>2;
                    unsigned int *base = (unsigned int *)CVPixelBufferGetBaseAddress(buf);
                                        
                    if(Touch::click) {
                        
                        for(int i=0; i<HEIGHT; i++) {
                            for(int j=0; j<WIDTH; j++) {
                                
                                unsigned int pixel = base[i*row+j];
                                
                                buffer[i*WIDTH+j] = (pixel&0xFF00FF00)|((pixel>>16)&0xFF)|((pixel&0xFF)<<16);
                            }
                        }
                    }
                }
                else {
                    
                    for(int k=0; k<WIDTH*HEIGHT; k++) this->buffer[k] = 0;
                    
                }
                
                CVPixelBufferUnlockBaseAddress(buf,0);
                
            },"v@:@@");
            this->delegate = [Delegate new];
            
            this->device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
            this->deviceInput = [AVCaptureDeviceInput deviceInputWithDevice:this->device error:NULL];
            
            this->settings = @{(id)kCVPixelBufferPixelFormatTypeKey:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]};
            this->dataOutput = [[AVCaptureVideoDataOutput alloc] init];
            this->dataOutput.videoSettings = this->settings;
            [this->dataOutput setSampleBufferDelegate:this->delegate queue:dispatch_get_main_queue()];
            
            //セッション作成
            this->session = [[AVCaptureSession alloc] init];
            [this->session addInput:this->deviceInput];
            [this->session addOutput:this->dataOutput];
            this->session.sessionPreset = AVCaptureSessionPreset1920x1080;
            
            // カメラの向きなどを設定する
            [this->session beginConfiguration];
            
            for(AVCaptureConnection *connection in [this->dataOutput connections] ) {
                for(AVCaptureInputPort *port in [connection inputPorts] ) {
                    if([[port mediaType] isEqual:AVMediaTypeVideo] ) {
                        this->videoConnection = connection;
                    }
                }
            }
            if([this->videoConnection isVideoOrientationSupported]) {
                [this->videoConnection setVideoOrientation:AVCaptureVideoOrientationPortrait];
            }
            
            [this->session commitConfiguration];
            [this->session startRunning];

#else
            
            int w;
            int h;
            int bpp;
                
            this->buffer = (unsigned int *)stb_image::stbi_load([FileManager::path(@"test.png") UTF8String],&w,&h,&bpp,4);
            
#endif

        }
        
        ~Vdig() {
            delete[] this->buffer;
        }
    
};
