#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace EnhancedScript {
// GTA III's SCM float literals are signed fixed-point int16 / 16, not IEEE.
// Only constants and initialized global variables are resolved; no bytecode
// execution and no attempt to guess a running thread's local variables.
inline bool Number(const uint8_t *data, size_t size, size_t globalsEnd,
    size_t &at, float &out, bool real)
{
    if (at >= size) return false;
    const uint8_t type = data[at++];
    const size_t width = type == 1 ? 4 : type == 4 ? 1 :
        (type == 2 || type == 5 || type == 6) ? 2 : 0;
    if (!width || at+width > size) return false;
    int32_t value = 0;
    if (type == 1) std::memcpy(&value, data+at, 4);
    else if (type == 4) value = int8_t(data[at]);
    else {
        int16_t small;
        std::memcpy(&small, data+at, 2);
        value = small;
    }
    at += width;
    if (type == 2) {
        const size_t offset = uint16_t(value);
        if (offset < 8 || offset+4 > globalsEnd || offset+4 > size) return false;
        if (real) std::memcpy(&out, data+offset, 4);
        else { std::memcpy(&value, data+offset, 4); out = float(value); }
    } else out = type == 6 ? float(value)/16.0f : float(value);
    return std::isfinite(out);
}
}
