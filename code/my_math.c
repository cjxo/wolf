#define trig_angle_count 360
static fixed_16_16 FixedSinTable[trig_angle_count];
static fixed_16_16 FixedCosTable[trig_angle_count];

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
        FixedSinTable[Angle] = f32_to_fixed_16_16(Result);
        
        Result = SineApprox(Radians + 3.1415926f*0.5f);
        FixedCosTable[Angle] = f32_to_fixed_16_16(Result);
    }
}