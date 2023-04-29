#if 0
#include "math.h"

// TODO: Will need to make these all platform-performant!!
// This is to remove math.h

inline i32 round_f32_to_i32(f32 real)
{
    return (i32)roundf(real);
}

inline u32 round_f32_to_u32(f32 real)
{
    return (u32)roundf(real);
}

inline i32 floor_f32_to_i32(f32 real)
{
    return (i32)floorf(real);
}

inline f32 sine(f32 angle)
{
    return sinf(angle);
}

inline f32 cosine(f32 angle)
{
    return cosf(angle);
}

inline f32 a_tan2(f32 y, f32 x)
{
    return atan2f(y, x);
}
#endif

// TODO: Eventually to use the intrinsic bit_scan
struct bit_scan_result
{
    b32 found;
    u32 index;
};

inline bit_scan_result find_least_sig_set_bit_32(u32 value)
{
    bit_scan_result result = {};
#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, value);
#else
    for (u32 test = 0; test < 32; ++test)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }

#endif

    return result;
}


