#define trig_angle_count 360
static f32 FixedSinTable[trig_angle_count];
static f32 FixedCosTable[trig_angle_count];

s32 _fltused;

static f32
SineApprox(f32 Radians)
{
    f32 t = Radians/(2*3.1415926f);
    t = t - (s32)t;
    if (t < 0.5f)
    {
        return -16.0f*(t*t-0.5f*t);
    }
    else
    {
        return 16.0f*(t-0.5f)*(t-1.0f);
    }
}

static void
Math_Init(void)
{
    for (u32 Angle = 0; Angle < trig_angle_count; ++Angle)
    {
        f32 Radians = (Angle * 3.1415926f)/180.0f;
        f32 Result = SineApprox(Radians);
        FixedSinTable[Angle] = Result;
        
        Result = SineApprox(Radians + 3.1415926f*0.5f);
        FixedCosTable[Angle] = Result;
    }
}

inline static f32
Math_Cos(u32 Degrees)
{
    Degrees %= 360;
    return FixedCosTable[Degrees];
}

inline static f32
Math_Sin(u32 Degrees)
{
    Degrees %= 360;
    return FixedSinTable[Degrees];
}

inline static f32
Math_Abs(f32 f)
{
    union { f32 f; u32 n; } a;
    a.f = f;
    a.n &= ~0x80000000;
    return(a.f);
}

inline static v2
V2(f32 X, f32 Y)
{
    v2 Result;
    Result.X = X;
    Result.Y = Y;
    return(Result);
}