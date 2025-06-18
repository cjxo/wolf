/* date = June 14th 2025 9:41 pm */

#ifndef MY_MATH_H
#define MY_MATH_H

#include <math.h>

static void Math_Init(void);
inline static f32 Math_Cos(u32 Degrees);
inline static f32 Math_Sin(u32 Degrees);
inline static f32 Math_Abs(f32 f);

typedef union
{
    struct
    {
        f32 X, Y;
    };
    f32 V[2];
} v2;

inline static v2 V2(f32 X, f32 Y);

#endif //MY_MATH_H