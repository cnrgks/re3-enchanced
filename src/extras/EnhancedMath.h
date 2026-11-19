#pragma once
#include <cmath>

namespace EnhancedMath {
inline double CursorCoordinate(double value, int windowExtent, int renderExtent)
{
    if (windowExtent<=0 || renderExtent<=0 || !std::isfinite(value)) return 0;
    double result=value*double(renderExtent)/double(windowExtent);
    if (result<0) result=0;
    if (result>renderExtent-1) result=renderExtent-1;
    return result;
}
inline int ScrollFirstVisible(int first, int selected, int count, int visible)
{
    if (count<=visible || visible<=0) return 0;
    if (selected<first) first=selected;
    if (selected>=first+visible) first=selected-visible+1;
    if (first<0) first=0;
    if (first>count-visible) first=count-visible;
    return first;
}
inline float RadarIconScale(float range)
{
    float scale=1.0f/std::sqrt(range>120.0f ? range/120.0f : 1.0f);
    return scale<0.35f ? 0.35f : scale;
}
inline float SmoothZoom(float current, float target, float elapsed)
{
    if (elapsed < 0.0f) elapsed=0.0f;
    if (elapsed > 0.1f) elapsed=0.1f;
    return current+(target-current)*(1.0f-std::exp(-7.0f*elapsed));
}

// Vector types only need x/y/z, allowing both RenderWare vertices and engine
// vectors without casts or dependence on a renderer in the geometry tests.
template<class Ray, class Vertex>
bool SegmentTriangle(const Ray &start, const Ray &delta,
    const Vertex &a, const Vertex &b, const Vertex &c, float &best)
{
    const float e1x=b.x-a.x,e1y=b.y-a.y,e1z=b.z-a.z;
    const float e2x=c.x-a.x,e2y=c.y-a.y,e2z=c.z-a.z;
    const float px=delta.y*e2z-delta.z*e2y;
    const float py=delta.z*e2x-delta.x*e2z;
    const float pz=delta.x*e2y-delta.y*e2x;
    const float determinant=e1x*px+e1y*py+e1z*pz;
    if (std::fabs(determinant)<0.000001f) return false;
    const float inverse=1.0f/determinant;
    const float sx=start.x-a.x,sy=start.y-a.y,sz=start.z-a.z;
    const float u=(sx*px+sy*py+sz*pz)*inverse;
    if (u<0.0f || u>1.0f) return false;
    const float qx=sy*e1z-sz*e1y;
    const float qy=sz*e1x-sx*e1z;
    const float qz=sx*e1y-sy*e1x;
    const float v=(delta.x*qx+delta.y*qy+delta.z*qz)*inverse;
    if (v<0.0f || u+v>1.0f) return false;
    const float t=(e2x*qx+e2y*qy+e2z*qz)*inverse;
    if (t<0.0f || t>best) return false;
    best=t;
    return true;
}
}
