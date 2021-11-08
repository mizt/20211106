#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <AVFoundation/AVFoundation.h>
#import <objc/runtime.h>
void addMethod(Class cls,NSString *method,id block,const char *type,bool isClassMethod=false) {
    SEL sel = NSSelectorFromString(method);
    int ret = ([cls respondsToSelector:sel])?1:(([[cls new] respondsToSelector:sel])?2:0);
    if(ret) {
        class_addMethod(cls,(NSSelectorFromString([NSString stringWithFormat:@"_%@",(method)])),method_getImplementation(class_getInstanceMethod(cls,sel)),type);
        class_replaceMethod((ret==1)?object_getClass((id)cls):cls,sel,imp_implementationWithBlock(block),type);
    }
    else {
        class_addMethod((isClassMethod)?object_getClass((id)cls):cls,sel,imp_implementationWithBlock(block),type);
    }
}

#import <vector>

#import "FileManager.h"
namespace FileManager {

    NSString *removePlatform(NSString *str) {
        NSString *extension = [NSString stringWithFormat:@".%@",[str pathExtension]];
        return FileManager::replace(str,@[
            [NSString stringWithFormat:@"-macosx%@",extension],
            [NSString stringWithFormat:@"-iphoneos%@",extension],
            [NSString stringWithFormat:@"-iphonesimulator%@",extension]],
            extension);
    }

    NSString *addPlatform(NSString *str) {
        NSString *extension = [NSString stringWithFormat:@".%@",[str pathExtension]];
#if TARGET_OS_OSX
        return FileManager::replace(FileManager::removePlatform(str),extension,[NSString stringWithFormat:@"-macosx%@",extension]);
#elif TARGET_OS_SIMULATOR
        return FileManager::replace(FileManager::removePlatform(str),extension,[NSString stringWithFormat:@"-iphonesimulator%@",extension]);
#elif TARGET_OS_IPHONE
        return FileManager::replace(FileManager::removePlatform(str),extension,[NSString stringWithFormat:@"-iphoneos%@",extension]);
#else
        return nil;
#endif
    }

};

namespace Config {
    const int WIDTH = 1080;
    const int HEIGHT = 1920;
    const int NUM = (32)<<1;
    const int BRUSH_SIZE = 128;
};

#define FPS 30.0

#import "Plane.h"
#import "MetalLayer.h"
#import "MetalUIWindow.h"
#import "MetalBaseLayer.h"
#import "ComputeShaderBase.h"

#import "Brush.h"
#import "Brusher.h"
#import "Paint.h"
#import "Mask.h"
#import "Feedback.h"

#import "Vdig.h"

class App {
    
    private:
    
        MetalView *_view = nil;
        MetalLayer<Plane> *_layer;
        dispatch_source_t _timer;
    
        bool isInit = false;
        Vdig *vdig = nullptr;
    
        Paint *_paint;
        Mask *_mask;
        Feedback *_feedback;
    
        unsigned int *_softbrush;
    
        int _brushsize = Config::BRUSH_SIZE;
        std::vector<Brusher *> _brusher;
        unsigned int *_brushStroke = nullptr;
    
        void appear() {
            NSLog(@"appear");
            [MetalUIWindow::$()->view() addSubview:this->_view];
            MetalUIWindow::$()->appear();
        }
    
        void clear(int n) {
            unsigned int *buffer = this->_feedback->bytes();
            
            this->_brusher[n]->update();
            
            int x = this->_brusher[n]->x()-this->_brushsize;
            int y = this->_brusher[n]->y()-this->_brushsize;
            
            for(int i=0; i<this->_brushsize; i++) {
                
                int i2 = i+y;
                
                if(i2>=0&&i2<=Config::HEIGHT-1) {
                for(int j=0; j<this->_brushsize; j++) {
                 
                    int j2 = j+x;
                    
                        if(j2>=0&&j2<=Config::WIDTH-1) {
                            
                            unsigned int c = this->_softbrush[i*this->_brushsize+j];
                            unsigned char a = c>>24;
                             
                            if(a>0) {
                                
                                int addr = i2*Config::WIDTH+j2;
                                
                                unsigned int pixel = this->vdig ->buffer[addr];
                                
                                if(a==255) {
                                    
                                    buffer[addr] = 0xFF000000|(pixel&0xFFFFFF);
                                }
                                else {
                                    
                                    float wet = a/255.0;
                                    float dry = 1.0-wet;
                                    
                                    unsigned int dst = buffer[addr];
                                    
                                    unsigned char r = ((pixel>>16)&0xFF)*wet + ((dst>>16)&0xFF)*dry;
                                    unsigned char g = ((pixel>>8)&0xFF)*wet  + ((dst>>8)&0xFF)*dry;
                                    unsigned char b = ((pixel)&0xFF)*wet + ((dst)&0xFF)*dry ;
                                   
                                    buffer[addr] = 0xFF000000|r<<16|g<<8|b;
                                    
                                }
                                   
                                float coeff = 0.1;
                                
                                float wet = (a/255.0)*coeff;
                                float dry = 1.0-wet;
                                
                                unsigned int tmp = this->_brushStroke[addr];
                                                           
                                int r = ((tmp>>16)&0xFFFF)*dry+0x8000*wet;
                                int g = (tmp&0xFFFF)*dry+0x8000*wet;
                                
                                 if(r<=0) r = 0;
                                 if(g<=0) g = 0;
                                 if(r>=0xFFFF) r = 0xFFFF;
                                 if(g>=0xFFFF) g = 0xFFFF;
                                 
                                 this->_brushStroke[addr] = r<<16|g;
                            }
                        }
                    }
                }
            }
        }

        void painting(int n) {
            
            double dx = this->_brusher[n]->x()-this->_brushsize;
            double dy = this->_brusher[n]->y()-this->_brushsize;
        
            this->_brusher[n]->update();
        
            dx -= this->_brusher[n]->x()-this->_brushsize;
            dy -= this->_brusher[n]->y()-this->_brushsize;
            
            int x = this->_brusher[n]->x()-this->_brushsize;
            int y = this->_brusher[n]->y()-this->_brushsize;

            for(int i=0; i<this->_brushsize; i++) {
                
                int i2 = i+y;
                
                if(i2>=0&&i2<=Config::HEIGHT-1) {
                    
                    for(int j=0; j<this->_brushsize; j++) {
                 
                        int j2 = j+x;
                    
                        if(j2>=0&&j2<=Config::WIDTH-1) {
                            
                            int addr = i2*Config::WIDTH+j2;
                            
                            unsigned int c = this->_softbrush[i*this->_brushsize+j];
                            unsigned char a = c>>24;
                             
                            if(a>0) {
                                
                                float coeff = a/255.0;
                                unsigned int tmp = this->_brushStroke[addr];
                                
                                int r = ((tmp>>16)&0xFFFF)+dx*16.0*coeff;
                                int g = (tmp&0xFFFF)+dy*16.0*coeff;
                               
                                if(r<=0) r = 0;
                                if(g<=0) g = 0;
                                if(r>=0xFFFF) r = 0xFFFF;
                                if(g>=0xFFFF) g = 0xFFFF;
                                
                                this->_brushStroke[addr] = r<<16|g;
                            }
                        }
                    }
                }
            }
        }

    public:
        
        App() {
            
            this->vdig = new Vdig();
            
            for(int k=0; k<Config::NUM; k++) {
                this->_brusher.push_back(new Brusher(Config::WIDTH+this->_brushsize*2,Config::HEIGHT+this->_brushsize*2));
            }
            
            this->_softbrush = (new Brush(this->_brushsize,this->_brushsize,FileManager::addPlatform(@"SoftBrush.metallib")))->exec();

            this->_paint = new Paint(Config::WIDTH,Config::HEIGHT,FileManager::addPlatform(@"Paint.metallib"));
          
            this->_mask = new Mask(Config::WIDTH,Config::HEIGHT,FileManager::addPlatform(@"Mask.metallib"));
            this->_feedback = new Feedback(Config::WIDTH,Config::HEIGHT,FileManager::addPlatform(@"Feedback.metallib"));
          
            this->_brushStroke = new unsigned int[Config::WIDTH*Config::HEIGHT];
            for(int i=0; i<Config::HEIGHT; i++) {
                for(int j=0; j<Config::WIDTH; j++) {
                    this->_brushStroke[i*Config::WIDTH+j] = 0x80008000; // ABGR
                }
            }
             
            CGRect rect = CGRectMake(0,0,Config::WIDTH,Config::HEIGHT);
            
            this->_view = [[MetalView alloc] initWithFrame:CGRectMake(0,0,rect.size.width,rect.size.height)];
            this->_view.backgroundColor = [UIColor blackColor];
            this->_layer = new MetalLayer<Plane>((CAMetalLayer *)this->_view.layer);
            if(this->_layer&&this->_layer->init(rect.size.width,rect.size.height,@"default.metallib")) {
                
                CGRect bounds = MetalUIWindow::$()->bounds();
                
                int h = Config::HEIGHT*(bounds.size.width/Config::WIDTH);
                
                this->_view.frame = CGRectMake(0,(((int)bounds.size.height)-h)>>1,bounds.size.width,h);
                
                this->_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("ENTER_FRAME",0));
                dispatch_source_set_timer(this->_timer,dispatch_time(0,0),(1.0/FPS)*1000000000,0);
                dispatch_source_set_event_handler(this->_timer,^{
                    
                    static dispatch_once_t oncePredicate;
                    dispatch_once(&oncePredicate,^{
                        dispatch_async(dispatch_get_main_queue(),^{
                            this->appear();
                        });
                    });
                    
                    if(!this->isInit) {
                        if(Touch::click) {
                            this->isInit = true;
                        }
                        else {
                            return;
                        }
                    }
                    
                    for(int k=0; k<(Config::NUM>>1); k++) {
                        this->clear(k*2+0);
                        this->painting(k*2+1);
                    }
                    
                    this->_feedback->exec(this->_brushStroke);
                    [this->_layer->texture() replaceRegion:MTLRegionMake2D(0,0,Config::WIDTH,Config::HEIGHT) mipmapLevel:0 withBytes:this->_feedback->bytes() bytesPerRow:Config::WIDTH<<2];
                    
                    this->_layer->update(^(id<MTLCommandBuffer> commandBuffer) {
                        this->_layer->cleanup();
                    });
                });
                if(this->_timer) dispatch_resume(this->_timer);
            }
            else {
                static dispatch_once_t oncePredicate;
                dispatch_once(&oncePredicate,^{
                    dispatch_async(dispatch_get_main_queue(),^{
                        this->appear();
                    });
                });
            }
        }
    
        ~App() {
            if(this->_timer) {
                dispatch_source_cancel(this->_timer);
                this->_timer = nullptr;
            }
            delete[] this->_brushStroke;
        }
    
};


@interface AppDelegate:UIResponder<UIApplicationDelegate> {
    App *app;
}
@end

@implementation AppDelegate

-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    app = new App();
    return YES;
}

-(void)applicationWillTerminate:(UIApplication *)application {
    if(app) delete app;
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc,argv,nil,@"AppDelegate");
    }
}
