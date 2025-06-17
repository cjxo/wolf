/* date = June 14th 2025 9:41 pm */

#ifndef MY_MATH_H
#define MY_MATH_H

#include <math.h>

typedef s32 fixed_16_16;
#define fixed_16_16_int(f) (((f)&(0xffff0000))>>16)
#define fixed_16_16_frac(f) ((f)&(0x0000ffff))
#define fixed_16_16_mul(a,b) (((a)>>8)*((b)>>8))
#define f32_to_fixed_16_16(f) (fixed_16_16)((f)*(float)(1<<16)+((f)>=0.0f?0.5f:-0.5f))

static void Math_Init(void);
static fixed_16_16 Math_Cos(u32 Degrees);
static fixed_16_16 Math_Sin(u32 Degrees);

typedef union
{
    struct
    {
        fixed_16_16 X;
        fixed_16_16 Y;
    };
    fixed_16_16 V[2];
} V2_Fixed;

#endif //MY_MATH_H