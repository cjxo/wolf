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
#define kb(v) ((u64)(v)*1024llu)
#define mb(v) (kb(v)*1024llu)
#define gb(v) (mb(v)*1024llu)
#define align(a,b) ((a)+((b)-1))&(~((b)-1))

#define dll_push_back_np(f,l,n,next,prev) (((f)==0)?((f)=(l)=(n)):((l)->next=(n),(n)->prev=(l),(l)=(n)))

#define true 1
#define false 0
#define null 0

#define invalid_default_case() default:{assert_break();}break
#define array_count(a) (sizeof(a)/sizeof((a)[0]))
#define forever while (true)

#define m_arena_default_commit_size mb(1)
typedef struct
{
    u8 *Base;
    u64 Capacity, StackPtr, CommitPtr;
} M_Arena;

#define m_arena_push_array(a,T,c) M_ArenaPush((a),sizeof(T)*(c))
#define m_arena_push(a,T) m_arena_push_array(a,T,1)
inline static M_Arena *M_ArenaReserve(u64 ReserveSize);
static void           *M_ArenaPush(M_Arena *Arena, u64 PushSize);
static b32             M_ArenaPop(M_Arena *Arena, u64 PushSize);

typedef struct
{
    u64 State;
} PRNG;

inline static void PRNG_Seed(PRNG *State, u64 Seed);
inline static u32  PRNG_U32(PRNG *State);
static u32         PRNG_RangeU32(PRNG *State, u32 Low, u32 High);
inline static f32  PRNG_NormF32(PRNG *State);

#define str8(s) (String_U8_Const){(u8*)(s),(sizeof(s)-1),(sizeof(s)-1)}
typedef struct
{
    u8 *S;
    u64 Cap, Count;
} String_U8_Const;
typedef String_U8_Const String_U8;

inline static String_U8_Const Str8_FromCSTR(char *CSTR);
static String_U8_Const        Str8_Copy(String_U8_Const Source, M_Arena *Arena);

inline static u64 CStr_Length(char *CSTR);
#endif //BASE_H