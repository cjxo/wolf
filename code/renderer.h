/* date = June 16th 2025 1:10 pm */

#ifndef RENDERER_H
#define RENDERER_H

typedef struct
{
    u8 *Pixels;
    s32 Width, Height;
    
    f32 *DepthBuffer1D;
} R_State;

// Always 4 bpp
typedef struct
{
    u8 *Pixels;
    s32 Width, Height;
} R_Texture2D;

static void R_Init(R_State *state);
static void R_LoadTexture(R_Texture2D *OutputTexture, char *Filename);
void R_WireRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour);
void R_FillRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour);
void R_VerticalLine(R_State *state, s32 X, s32 YStart, s32 YEnd, u32 Colour);
static void R_VerticalLineFromTexture2D(R_State *State, s32 X, s32 YStart, s32 YEnd, R_Texture2D Texture, s32 TextureX);

#endif //RENDERER_H
