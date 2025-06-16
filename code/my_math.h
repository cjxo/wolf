/* date = June 14th 2025 9:41 pm */

#ifndef MY_MATH_H
#define MY_MATH_H

#include <math.h>

typedef u32 fixed_16_16;
#define fixed_16_16_int(f) (((f)&(0xffff0000))>>16)
#define fixed_16_16_frac(f) ((f)&(0x0000ffff))
#define f32_to_fixed_16_16(f) (fixed_16_16)((f)*(float)(1<<16)+((f)>=0.0f?0.5f:-0.5f))

static void Math_Init(void);

#endif //MY_MATH_H