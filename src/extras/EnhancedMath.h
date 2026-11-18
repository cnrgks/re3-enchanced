#pragma once
#include <cmath>

namespace EnhancedMath {
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
