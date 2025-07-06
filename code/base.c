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

inline static M_Arena *
M_ArenaReserve(u64 ReserveSize)
{
    M_Arena *Result = 0;
    ReserveSize = align(ReserveSize, 16);
    u64 InitialCommit = align(sizeof(M_Arena), 16);
    void *Block = VirtualAlloc(0, ReserveSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (Block)
    {
        Result = Block;
        Result->Base = Block;
        Result->Capacity = ReserveSize;
        Result->StackPtr = InitialCommit;
        Result->CommitPtr = InitialCommit;
    }
    w_assert(Result != 0);
    return(Result);
}

static void *
M_ArenaPush(M_Arena *Arena, u64 PushSize)
{
    void *Result = 0;
    PushSize = align(PushSize, 16);
    if ((Arena->StackPtr + PushSize) <= Arena->Capacity)
    {
        Result = Arena->Base + Arena->StackPtr;
        Arena->StackPtr += PushSize;
    }
    
    w_assert(Result != 0);
    return(Result);
}

static b32
M_ArenaPop(M_Arena *Arena, u64 PushSize)
{
    b32 Result = false;
    PushSize = align(PushSize, 16);
    
    u64 InitialCommit = align(sizeof(M_Arena), 16);
    if (Arena->StackPtr >= (InitialCommit + PushSize))
    {
        Result = true;
        Arena->StackPtr -= PushSize;
    }
    return(Result);
}

inline static u64
CStr_Length(char *CSTR)
{
    u64 Result = 0;
    while (*(CSTR++))
    {
        Result += 1;
    }
    return(Result);
}

inline static String_U8_Const
Str8_FromCSTR(char *CSTR)
{
    String_U8_Const Result;
    Result.S = (u8 *)CSTR;
    Result.Cap = CStr_Length(CSTR);
    Result.Count = Result.Cap;
    return(Result);
}

static String_U8_Const
Str8_Copy(String_U8_Const Source, M_Arena *Arena)
{
    String_U8_Const Result;
    Result.S = m_arena_push_array(Arena, u8, Source.Cap);
    Result.Cap = Source.Cap;
    Result.Count = Source.Count;
    CopyMemory(Result.S, Source.S, Result.Count);
    return(Result);
}