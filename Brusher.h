class Brusher {
  
    private:
    
        float damping = 0.99;
        
        int w = 0;
        int h = 0;
    
        double vx = 0;
        double vy = 0;
    
        double mx = 0;
        double my = 0;
        
        double sx = 0.0;
        double sy = 0.0;
    
        double scale = 333.0;
        double clip = 2.5;
    
    public:
        
        Brusher(int w,int h) {
            
            this->w = w;
            this->h = h;
            
            this->mx = random()%w;
            this->my = random()%h;
            
            this->sx = w/this->scale;
            this->sy = h/this->scale;
        }
    
        ~Brusher() {
        }
    
        double x() {
            return this->mx;
        }
    
        double y() {
            return this->my;
        }
        
        float rand() {
            return ((random()/(float)(RAND_MAX))-0.5)*2.0;
        }
    
        void update() {
            
            this->vx += this->rand() * this->sx;
            this->vy += this->rand() * this->sy;
            
            if(this->vx>this->sx*this->clip) this->vx = this->sx*this->clip;
            if(this->vy>this->sy*this->clip) this->vy = this->sy*this->clip;

            if(this->vx<-this->sx*this->clip) this->vx = -this->sx*this->clip;
            if(this->vy<-this->sy*this->clip) this->vy = -this->sy*this->clip;
            
            this->vx*=damping;
            this->vy*=damping;
            
            this->mx+=this->vx;
            this->my+=this->vy;
            
            if(this->mx>=this->w) this->mx = 0;
            else if(this->mx<0) this->mx = (this->w-1);
            
            if(this->my>=this->h) this->my = 0;
            else if(this->my<0) this->my = (this->h-1);
            
        }
};
