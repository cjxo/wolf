static void
R_Init(R_State *state)
{
    state->Width = 1280;
    state->Height = 720;
    u64 Size = 1280*720*4;
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