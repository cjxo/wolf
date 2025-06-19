static void
R_Init(R_State *state)
{
    state->Width = 640;
    state->Height = 360;
    u64 Size = state->Width*state->Height*4;
    state->Pixels = VirtualAlloc(0, Size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}

#if 0
static void
R_WireRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour)
{
    s32 XStart, YStart, XEnd, YEnd, LoopVar, XEnd2, YEnd2, Pitch;
    u8 Alpha, Red, Green, Blue;
    u8 *Pixels, *LoopPixels;
    
    XStart = X;
    YStart = Y;
    XEnd = XEnd2 = X + W;
    YEnd = YEnd2 = Y + H;
    
    if ((X < state->Width) && (Y < state->Height) &&
        (XEnd >= 0) && (YEnd >= 0) && (W > 0) && (H > 0))
    {
        if (X < 0)
        {
            XStart = 0;
        }
        
        if (XEnd >= state->Width)
        {
            XEnd = state->Width - 1;
        }
        
        if (Y < 0)
        {
            YStart = 0;
        }
        
        if (YEnd >= state->Height)
        {
            YEnd = state->Height - 1;
        }
        
        Pitch = state->Width*4;
        Alpha = (Colour >> 24) & 0xff;
        Red = (Colour >> 16) & 0xff;
        Green = (Colour >> 8) & 0xff;
        Blue = (Colour >> 0) & 0xff;
        
        Pixels = state->Pixels + ((YStart*state->Width+XStart)*4);
        
        if (Y >= 0)
        {
            LoopPixels = Pixels;
            for (LoopVar = XStart; LoopVar <= XEnd; ++LoopVar)
            {
                LoopPixels[0] = Blue;
                LoopPixels[1] = Green;
                LoopPixels[2] = Red;
                LoopPixels[3] = Alpha;
                
                LoopPixels += 4;
            }
        }
        
        if (X >= 0)
        {
            LoopPixels = Pixels;
            for (LoopVar = YStart; LoopVar <= YEnd; ++LoopVar)
            {
                LoopPixels[0] = Blue;
                LoopPixels[1] = Green;
                LoopPixels[2] = Red;
                LoopPixels[3] = Alpha;
                
                LoopPixels += Pitch;
            }
        }
        
        if (XEnd2 < state->Width)
        {
            LoopPixels = Pixels + (XEnd - XStart)*4;
            for (LoopVar = YStart; LoopVar <= YEnd; ++LoopVar)
            {
                LoopPixels[0] = Blue;
                LoopPixels[1] = Green;
                LoopPixels[2] = Red;
                LoopPixels[3] = Alpha;
                
                LoopPixels += Pitch;
            }
        }
        
        if (YEnd2 < state->Height)
        {
            LoopPixels = Pixels + (YEnd - YStart)*Pitch;
            for (LoopVar = XStart; LoopVar <= XEnd; ++LoopVar)
            {
                LoopPixels[0] = Blue;
                LoopPixels[1] = Green;
                LoopPixels[2] = Red;
                LoopPixels[3] = Alpha;
                
                LoopPixels += 4;
            }
        }
    }
}
#endif

#if 0
static void
R_FillRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour)
{
    s32 XStart, YStart, XEnd, YEnd, Pitch, LoopVar;
    u8 Alpha, Red, Green, Blue;
    u8 *Pixels, *RowPixels;
    
    XStart = X;
    YStart = Y;
    XEnd = X + W;
    YEnd = Y + H;
    
    if (X < 0)
    {
        XStart = 0;
    }
    
    if (XEnd >= state->Width)
    {
        XEnd = state->Width - 1;
    }
    
    if (Y < 0)
    {
        YStart = 0;
    }
    
    if (YEnd >= state->Height)
    {
        YEnd = state->Height - 1;
    }
    
    Pitch = state->Width*4;
    Alpha = (Colour >> 24) & 0xff;
    Red = (Colour >> 16) & 0xff;
    Green = (Colour >> 8) & 0xff;
    Blue = (Colour >> 0) & 0xff;
    
    Pixels = state->Pixels + 4*(YStart*state->Width+XStart);
    for (; YStart <= YEnd; ++YStart)
    {
        RowPixels = Pixels;
        for (LoopVar = XStart; LoopVar <= XEnd; ++LoopVar)
        {
            RowPixels[0] = Blue;
            RowPixels[1] = Green;
            RowPixels[2] = Red;
            RowPixels[3] = Alpha;
            RowPixels += 4;
        }
        
        Pixels += Pitch;
    }
}
#endif

#if 0
static void
R_VerticalLine(R_State *State, s32 X, s32 YStart, s32 YEnd, u32 Colour)
{
    if ((X >= 0) && (X < State->Width))
    {
        if (YStart > YEnd)
        {
            s32 Temp = YStart;
            YStart = YEnd;
            YEnd = YStart;
        }
        
        if (YStart < 0) YStart = 0;
        else if (YStart > State->Height) YStart = State->Height;
        
        if (YEnd < 0) YEnd = 0;
        else if (YEnd > State->Height) YEnd = State->Height;
        
        u32 *OutPixels = (u32 *)(State->Pixels) + (YStart * State->Width + X);
        while (YStart < YEnd)
        {
            *OutPixels = Colour;
            OutPixels += State->Width;
            ++YStart;
        }
    }
}
#endif

static void
R_LoadTexture(R_Texture2D *OutputTexture, char *Filename)
{
    s32 NumComps;
    OutputTexture->Pixels = stbi_load(Filename, &(OutputTexture->Width), &(OutputTexture->Height), &NumComps, 4);
    w_assert(OutputTexture->Pixels!=0);
}

static void
R_VerticalLineFromTexture2D(R_State *State, s32 X, s32 YStart, s32 YEnd, R_Texture2D Texture, s32 TextureX)
{
    if ((X >= 0) && (X < State->Width))
    {
        if (YStart > YEnd)
        {
            s32 Temp = YStart;
            YStart = YEnd;
            YEnd = YStart;
        }
        
        f32 OldYStart = (f32)YStart;
        f32 Height = ((f32)YEnd - (f32)(YStart));
        
        if (YStart < 0) YStart = 0;
        else if (YStart >= State->Height) YStart = State->Height-1;
        
        if (YEnd < 0) YEnd = 0;
        else if (YEnd >= State->Height) YEnd = State->Height-1;
        
        if (TextureX < 0) TextureX = 0;
        else if (TextureX >= Texture.Width) TextureX = Texture.Width - 1;
        
        u32 *OutPixels = (u32 *)(State->Pixels) + (YStart * State->Width + X);
        f32 Step = 1.0f / Height;
        f32 V = ((f32)YStart - OldYStart) / Height;
        while (YStart <= YEnd)
        {
            u32 Colour = *(((u32 *)Texture.Pixels) + (((s32)(V*Texture.Height)*Texture.Width) + TextureX));
            
            u8 R = Colour & 0xFF;
            u8 G = (Colour>>8) & 0xFF;
            u8 B = (Colour>>16) & 0xFF;
            u8 A = (Colour>>24) & 0xFF;
            *OutPixels = (A<<24)|(R<<16)|(G<<8)|(B<<0);
            OutPixels += State->Width;
            ++YStart;
            V += Step;
        }
    }
}