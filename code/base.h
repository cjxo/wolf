/* date = June 14th 2025 9:01 pm */

#ifndef BASE_H
#define BASE_H

#include <stdint.h>
typedef   int8_t  s8;
typedef  uint8_t  u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;
typedef  int64_t s64;
typedef uint64_t u64;
typedef    float f32;
typedef   double f64;

typedef s32 b32;

#define assert_break() __debugbreak()
#if defined(WOLF_DEBUG)
# define w_assert(c) \
do\
{\
if(!(c))\
{\
assert_break();\
}\
}while(0)
#else
# define w_assert(c) (void)(c)
#endif

#define swap(a,b,T) \
do{\
T temp = a;\
a = b;\
b = temp;\
}while(0)

#define ror(r,c) _rotr((r),(c))

#define true 1
#define false 0
#define null 0

#define invalid_default_case() default:{assert_break();}break
#define array_count(a) (sizeof(a)/sizeof((a)[0]))
#define forever while (true)

typedef struct
{
    u64 State;
} PRNG;

inline static void PRNG_Seed(PRNG *State, u64 Seed);
inline static u32  PRNG_U32(PRNG *State);
static u32         PRNG_RangeU32(PRNG *State, u32 Low, u32 High);
inline static f32  PRNG_NormF32(PRNG *State);

#endif //BASE_H