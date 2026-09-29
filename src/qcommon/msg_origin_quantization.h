#pragma once

#include <cmath>

// Retail MP 006173C0/0040C900 and 0086FCC0/0086FD90 use
// FLD float; FADD double bias; FISTP int, also used by field equality.
// A C++ cast truncates instead and corrupts negative snapshot baselines.
inline int MSG_QuantizeOrigin(float value)
{
    const double bias = 9.313225746154785e-10;
#if defined(_MSC_VER) && defined(_M_IX86)
    int result;
    __asm
    {
        fld value
        fadd bias
        fistp result
    }
    return result;
#else
    return static_cast<int>(std::lrint(static_cast<double>(value) + bias));
#endif
}
