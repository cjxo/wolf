/* date = June 16th 2025 1:10 pm */

#ifndef RENDERER_H
#define RENDERER_H

typedef struct
{
    u8 *Pixels;
    s32 Width;
    s32 Height;
} R_State;

static void R_Init(R_State *state);
static void R_WireRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour);
static void R_FillRectangle(R_State *state, s32 X, s32 Y, s32 W, s32 H, u32 Colour);

#endif //RENDERER_H
