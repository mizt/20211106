#if !TARGET_OS_OSX

    @interface MetalView:UIView @end
    @implementation MetalView
        +(Class)layerClass { return [CAMetalLayer class]; }
    @end
    
#endif

@interface MetalViewController:UIViewController {
    void (^load)();
} @end
@implementation MetalViewController
-(BOOL)prefersStatusBarHidden { return YES; }
@end

namespace Touch {
    float x = 0;
    float y = 0;
    double begin = CFAbsoluteTimeGetCurrent();
    int click = 0;
}

@interface TouchScrollView:UIScrollView @end
@implementation TouchScrollView

    -(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
        Touch::begin = CFAbsoluteTimeGetCurrent();
        
        Touch::click = 1;

        CGPoint locationPoint = [[touches anyObject] locationInView:self];
        Touch::x = locationPoint.x;
        Touch::y = locationPoint.y;
        
    }
    -(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
        
        CGPoint locationPoint = [[touches anyObject] locationInView:self];
        Touch::x = locationPoint.x;
        Touch::y = locationPoint.y;
        
    }
    
    -(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
        double deleta = CFAbsoluteTimeGetCurrent()-Touch::begin;
        if(deleta<0.1) {
            Touch::click++;
            if(Touch::click==2) {
                self.scrollEnabled = YES;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(),^{
                    Touch::click = 0;
                    self.scrollEnabled = NO;
                });
            }
        }
        else {
            Touch::click = 0;
        }

        CGPoint locationPoint = [[touches anyObject] locationInView:self];
        Touch::x = locationPoint.x;
        Touch::y = locationPoint.y;
    }
    
@end

@interface MetalScrollViewController:UIViewController {
    void (^load)();
} @end
@implementation MetalScrollViewController
-(BOOL)prefersStatusBarHidden { return YES; }
-(void)viewDidLoad {
    [super viewDidLoad];
    
    load = nil;
    
    TouchScrollView *view = [[TouchScrollView alloc] initWithFrame:self.view.bounds];
    
    view.contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentNever;
    view.contentInset = UIEdgeInsetsMake(0.0,0.0,0.0,0.0);
    view.showsHorizontalScrollIndicator = NO;
    view.showsVerticalScrollIndicator = NO;
    
    view.refreshControl = [[UIRefreshControl alloc] init];
    [view.refreshControl addTarget:self action:@selector(refreshing:) forControlEvents:UIControlEventValueChanged];
    
    view.scrollEnabled = NO;
    
    self.view = view;
    self.view.multipleTouchEnabled = false;
}

-(void)callback:(void (^)())func {
    load = func;
}

-(void)refreshing:(UIRefreshControl *)sender {
    TouchScrollView *view = (TouchScrollView *)self.view;
    
    if(load) load();
    
    [view.refreshControl endRefreshing];
}
@end

class ViewController {
    
    protected:
        
        MetalViewController *_viewcontroller = nil;
    
    public:

        MetalViewController *viewController() {
            return this->_viewcontroller;
        }
};

class SliderViewController {
    
    protected:
        
        MetalScrollViewController *_viewcontroller = nil;
    
    public:

        MetalScrollViewController *viewController() {
            return this->_viewcontroller;
        }
};

class MetalUIWindow : public SliderViewController {
    
    private:
        
        UIWindow *_window = nil;
        float _scaleFactor;
    
        CGRect _bounds = [[UIScreen mainScreen] bounds];
    
        MetalUIWindow() {
                        
            this->_scaleFactor = [UIScreen mainScreen].scale;
            this->_window = [[UIWindow alloc] initWithFrame:this->_bounds];
            this->_viewcontroller = [[MetalScrollViewController alloc] init];
            [this->_window setRootViewController:this->_viewcontroller];
        }
    
        ~MetalUIWindow() = default;

    public:
    
        UIWindow *window() {
            return this->_window;
        }
    
        UIView *view() {
            return this->_viewcontroller.view;
        }
    
        CGRect bounds() {
            return  this->_bounds;
        }
    
        float scaleFactor() {
            return this->_scaleFactor;
        }
    
        void appear() {
            [this->_window makeKeyAndVisible];
        }
        
        static MetalUIWindow *$() {
            static MetalUIWindow *instance = nullptr;
            if(instance==nullptr) {
                instance = new MetalUIWindow();
            }
            return instance;
        }
};
