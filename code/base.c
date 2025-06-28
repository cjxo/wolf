#define PCG32_default_multiplier 6364136223846793005ULL
#define PCG32_default_increment  1442695040888963407ULL

inline static void
PRNG_Seed(PRNG *State, u64 Seed)
{
    State->State = 0;
    PRNG_U32(State);
    State->State += Seed;
    PRNG_U32(State);
}

inline static u32
PRNG_U32(PRNG *State)
{
    u64 S = State->State;
    State->State = S*PCG32_default_multiplier + PCG32_default_increment;
    u32 Value = (u32)((S^(S>>18))>>27);
    s32 Rot = S >> 59;
    return ror(Value, Rot);
}

static u32
PRNG_RangeU32(PRNG *State, u32 Low, u32 High)
{
    u32 Bound = High - Low;
    
    u64 M = (u64)PRNG_U32(State) * (u64)Bound;
    u64 L = (u32)M;
    
    if (L < M)
    {
        u32 T = -(s32)Bound%Bound;
        while (L < T)
        {
            M = (u64)PRNG_U32(State) * (u64)Bound;
            L = (u32)M;
        }
    }
    
    u32 Result = Low + (u32)(M>>32);
    return(Result);
}

inline static f32
PRNG_NormF32(PRNG *State)
{
    u32 X = PRNG_U32(State);
    f32 Result = (f32)((s32)(X>>8))*0x1.0p-24f;
    return(Result);
}