#include "../src/extras/EnhancedMath.h"
#include <cstdio>
#include <cstdlib>
struct Point { float x,y,z; };
void require(bool condition,const char *message)
{
    if (!condition) { std::fprintf(stderr,"FAILED: %s\n",message); std::exit(1); }
}
int main()
{
    const Point a={-1,-1,0},b={1,-1,0},c={0,1,0};
    const Point start={0,0,1},delta={0,0,-2};
    float t=1;
    require(EnhancedMath::SegmentTriangle(start,delta,a,b,c,t),"front-face windscreen hit");
    require(std::fabs(t-0.5f)<0.00001f,"correct hit parameter");
    t=1;
    require(EnhancedMath::SegmentTriangle(start,delta,c,b,a,t),"back-face windscreen hit");
    t=1;
    require(!EnhancedMath::SegmentTriangle(Point{3,0,1},delta,a,b,c,t),"miss outside pane");
    require(t==1,"miss must not corrupt nearest hit");
    require(!EnhancedMath::SegmentTriangle(start,Point{0,0,-0.5f},a,b,c,t),"pane beyond impact/wall endpoint");
    require(!EnhancedMath::SegmentTriangle(start,Point{1,0,0},a,b,c,t),"parallel segment");
    require(!EnhancedMath::SegmentTriangle(start,delta,a,a,a,t),"degenerate pane");
    require(!EnhancedMath::SegmentTriangle(Point{0,0,-1},delta,a,b,c,t),"intersection behind source");
    t=0.25f;
    require(!EnhancedMath::SegmentTriangle(start,delta,a,b,c,t),"far pane cannot replace nearer hit");
    require(t==0.25f,"keep nearest hit parameter");
    float at30=1,at144=1;
    for (int i=0;i<30;++i) at30=EnhancedMath::SmoothZoom(at30,3,1.0f/30);
    for (int i=0;i<144;++i) at144=EnhancedMath::SmoothZoom(at144,3,1.0f/144);
    require(std::fabs(at30-at144)<0.00002f,"zoom independent of frame rate");
    require(at30>1 && at30<3,"zoom approaches target without instant jump/overshoot");
    float zoom=3;
    for (int i=0;i<60;++i) {
        const float next=EnhancedMath::SmoothZoom(zoom,1,1.0f/60);
        require(next<=zoom && next>=1,"release converges monotonically"); zoom=next;
    }
    require(EnhancedMath::SmoothZoom(2,3,0)==2,"zero elapsed time");
    require(EnhancedMath::SmoothZoom(2,3,-1)==2,"negative elapsed time");
    require(EnhancedMath::SmoothZoom(1,3,10)<2.1f,"stall delta capped");
    std::puts("PASS: windscreen segment bounds, occlusion, nearest hit, and frame-rate independent zoom");
}
