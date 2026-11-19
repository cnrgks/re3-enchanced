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
}
