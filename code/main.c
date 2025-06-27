// NOTE(cj): TODOs
// - [ ] Multiple Sprites
//    - [ ] Sorting Sprites with respect to cam
// - [ ] Random Dungeon Generation
//    - We will add roguelike elements here!

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./ext/stb_image.h"

#include "base.h"
#include "renderer.h"
#include "my_math.h"

#include "my_math.c"
#include "renderer.c"

typedef u8 INP_Flags;
enum
{
    INP_Flag_KeyPressed = 0x1,
    INP_Flag_KeyReleased = 0x2,
    INP_Flag_KeyHeld = 0x4,
    INP_Flag_KeyNatural = 0x8,
};

typedef u8 INP_Key;
enum
{
    INP_Key_Up,
    INP_Key_Down,
    INP_Key_Left,
    INP_Key_Right,
    INP_Key_Q,
    INP_Key_E,
    INP_Key_Count,
};

typedef struct
{
    HWND WindowHandle;
    s32 WindowWidth;
    s32 WindowHeight;
    f32 AspectRatio;
    INP_Flags KeyFlags[INP_Key_Count];
} W32_State;

static LRESULT __stdcall
W32_WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    
    if (Message == WM_NCCREATE)
    {
        SetWindowLongPtrA(Window, GWLP_USERDATA, (LONG_PTR)(((CREATESTRUCT *)LParam)->lpCreateParams));
        return DefWindowProc(Window, Message, WParam, LParam);
    }
    
    W32_State *W32State = (W32_State *)GetWindowLongPtrA(Window, GWLP_USERDATA);
    if (!W32State)
    {
        return DefWindowProc(Window, Message, WParam, LParam);
    }
    
    switch (Message)
    {
        case WM_CLOSE:
        {
            DestroyWindow(Window);
        } break;
        
        case WM_DESTROY:
        {
            ExitProcess(0);
        } break;
        
        case WM_SIZE:
        {
            W32State->WindowWidth = LOWORD(LParam);
            W32State->WindowHeight = HIWORD(LParam);
            W32State->AspectRatio = (f32)W32State->WindowWidth/(f32)W32State->WindowHeight;
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

static INP_Key
W32_WPARAMToINP_Key(WPARAM WParam)
{
    INP_Key Result;
    switch (WParam)
    {
        case VK_UP:
        {
            Result = INP_Key_Up;
        } break;
        
        case VK_DOWN:
        {
            Result = INP_Key_Down;
        } break;
        
        case VK_LEFT:
        {
            Result = INP_Key_Left;
        } break;
        
        case VK_RIGHT:
        {
            Result = INP_Key_Right;
        } break;
        
        case 'E':
        {
            Result = INP_Key_E;
        } break;
        
        case 'Q':
        {
            Result = INP_Key_Q;
        } break;
        
        default:
        {
            Result = INP_Key_Count;
        } break;
    }
    return(Result);
}

static void
W32_Init(W32_State *state)
{
    WNDCLASSEXA WindowClass =
    {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = W32_WindowProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = GetModuleHandleA(null),
        .hIcon = LoadIconA(0, IDI_APPLICATION),
        .hCursor = LoadCursorA(0, IDC_ARROW),
        .hbrBackground = GetStockObject(BLACK_BRUSH),
        .lpszMenuName = 0,
        .lpszClassName = "Wolfenstein_Clone_Class",
        .hIconSm = 0,
    };
    
    if (!RegisterClassExA(&WindowClass))
    {
        // TODO(cj): Logging
        w_assert(!"RegisterClassExA failed.");
    }
    
    RECT ClientRect;
    ClientRect.left = 0;
    ClientRect.top = 0;
    ClientRect.right = 1280;
    ClientRect.bottom = 720;
    s32 Adjusted = AdjustWindowRectEx(&ClientRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_NOREDIRECTIONBITMAP);
    w_assert(Adjusted != 0);
    
    s32 WindowWidth = ClientRect.right - ClientRect.left;
    s32 WindowHeight = ClientRect.bottom - ClientRect.top;
    HWND WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "Wolfenstein", WS_OVERLAPPEDWINDOW, 0, 0, WindowWidth, WindowHeight, 0, 0, WindowClass.hInstance, state);
    
    if (!IsWindow(WindowHandle))
    {
        // TODO(cj): Logging
        w_assert(!"CreateWindowExA failed.");
    }
    
    state->WindowHandle = WindowHandle;
    state->WindowWidth = 1280;
    state->WindowHeight = 720;
    ShowWindow(WindowHandle, SW_SHOW);
}

void main(void)
{
    SetProcessDPIAware();
    timeBeginPeriod(1);
    
    W32_State W32State={0};
    R_State RState;
    
    W32_Init(&W32State);
    R_Init(&RState);
    Math_Init();
    
    DEVMODE DevMode;
    EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &DevMode);
    u64 RefreshRate = DevMode.dmDisplayFrequency;
    u64 MSPerFrame = 1000 / RefreshRate;
    f32 GameUpdateS = (1.0f / (f32)MSPerFrame);
    
    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    
    LARGE_INTEGER BeginPerformanceCounter;
    QueryPerformanceCounter(&BeginPerformanceCounter);
    
    s32 GameGrid[16*16] = 
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    
    v2 PlayerP = V2(8, 8);
    f32 PlayerDegreesRot = 0;
    f32 RotRate = 40.0f*GameUpdateS;
    
    R_Texture2D GreyStone;
    R_LoadTexture(&GreyStone, "..\\data\\textures\\greystone.png");
    
    R_Texture2D Sprites[1];
    R_LoadTexture(Sprites + 0, "..\\data\\textures\\barrel.png");
    
    R_Texture2D Textures[2];
    R_LoadTexture(Textures + 0, "..\\data\\textures\\eagle.png");
    R_LoadTexture(Textures + 1, "..\\data\\textures\\redbrick.png");
    
    f32 Fov = 66.0f * (3.14159f/180.0f);
    f32 Right = tanf(Fov*0.5f);
    
    forever
    {
        MSG Message;
        for (u32 Key = 0; Key < INP_Key_Count; ++Key)
        {
            W32State.KeyFlags[Key] &= ~(INP_Flag_KeyPressed|INP_Flag_KeyReleased);
        }
        
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_KEYUP:
                {
                    INP_Key key = W32_WPARAMToINP_Key(Message.wParam);
                    if (key != INP_Key_Count)
                    {
                        W32State.KeyFlags[key] |= INP_Flag_KeyReleased;
                        W32State.KeyFlags[key] &= ~INP_Flag_KeyHeld;
                    }
                } break;
                
                case WM_KEYDOWN:
                {
                    INP_Key key = W32_WPARAMToINP_Key(Message.wParam);
                    if (key != INP_Key_Count)
                    {
                        W32State.KeyFlags[key] |= (INP_Flag_KeyPressed|INP_Flag_KeyHeld);
                    }
                } break;
                
                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } break;
            }
        }
        
        // apply rotation matrixs
        if (W32State.KeyFlags[INP_Key_Left] & INP_Flag_KeyHeld)
        {
            PlayerDegreesRot -= RotRate;
            if (PlayerDegreesRot < 0)
            {
                PlayerDegreesRot = 360;
            }
        }
        
        if (W32State.KeyFlags[INP_Key_Right] & INP_Flag_KeyHeld)
        {
            PlayerDegreesRot += RotRate;
            if (PlayerDegreesRot>360)
            {
                PlayerDegreesRot = 0;
            }
        }
        
        f32 RequestMoveX = 0;
        f32 RequestMoveY = 0;
        
        // NOTE(cj): These vectors are in world coordinate system
        // These two vectors can be made into a matrix that transforms the camera to world space.
        v2 PlayerDir = V2(Math_Cos((u32)PlayerDegreesRot),Math_Sin((u32)PlayerDegreesRot));
        v2 PlayerCameraDir = V2(-PlayerDir.Y*Right, PlayerDir.X*Right);
        
        if (W32State.KeyFlags[INP_Key_Up] & INP_Flag_KeyHeld)
        {
            RequestMoveX = PlayerDir.X * 1 * GameUpdateS;
            RequestMoveY = PlayerDir.Y * 1 * GameUpdateS;
        }
        
        if (W32State.KeyFlags[INP_Key_Q] & INP_Flag_KeyHeld)
        {
            RequestMoveX = PlayerDir.Y * 1 * GameUpdateS;
            RequestMoveY = -PlayerDir.X * 1 * GameUpdateS;
        }
        
        if (W32State.KeyFlags[INP_Key_E] & INP_Flag_KeyHeld)
        {
            RequestMoveX = -PlayerDir.Y * 1 * GameUpdateS;
            RequestMoveY = PlayerDir.X * 1 * GameUpdateS;
        }
        
        if (W32State.KeyFlags[INP_Key_Down] & INP_Flag_KeyHeld)
        {
            RequestMoveX = -PlayerDir.X * 1 * GameUpdateS;
            RequestMoveY = -PlayerDir.Y * 1 * GameUpdateS;
        }
        
        if (RequestMoveY || RequestMoveX)
        {
            s32 OldXIDX = (s32)PlayerP.X;
            s32 OldYIDX = (s32)PlayerP.Y;
            
            f32 X = PlayerP.X + RequestMoveX;
            f32 Y = PlayerP.Y + RequestMoveY;
            
            s32 NewXIDX = (s32)X;
            s32 NewYIDX = (s32)Y;
            
            if ((NewXIDX < 16) && (NewYIDX < 16) && (NewXIDX >= 0) && (NewYIDX >= 0))
            {
                if (GameGrid[NewYIDX*16+NewXIDX] == 0)
                {
                    PlayerP.X = X;
                    PlayerP.Y = Y;
                }
            }
        }
        
        v2 RayDir0 = V2(PlayerDir.X - PlayerCameraDir.X, PlayerDir.Y - PlayerCameraDir.Y);
        v2 RayDir1 = V2(PlayerDir.X + PlayerCameraDir.X, PlayerDir.Y + PlayerCameraDir.Y);
        f32 PosZ = RState.Height*0.5f;
        for (s32 ScreenY = RState.Height/2; ScreenY < RState.Height; ++ScreenY)
        {
            s32 YPFromCenter = ScreenY - RState.Height/2;
            f32 RowDistance = PosZ/(f32)YPFromCenter;
            f32 FloorStepX = RowDistance*(RayDir1.X - RayDir0.X)/(f32)RState.Width;
            f32 FloorStepY = RowDistance*(RayDir1.Y - RayDir0.Y)/(f32)RState.Width;
            f32 FloorX = PlayerP.X + RowDistance*RayDir0.X;
            f32 FloorY = PlayerP.Y + RowDistance*RayDir0.Y;
            for (s32 ScreenX = 0; ScreenX < RState.Width; ++ScreenX)
            {
                s32 CellX = (s32)FloorX;
                s32 CellY = (s32)FloorY;
                
                s32 TexelX = (s32)(GreyStone.Width*(FloorX-(f32)CellX)) % GreyStone.Width;
                s32 TexelY = (s32)(GreyStone.Height*(FloorY-(f32)CellY)) % GreyStone.Height;
                
                FloorX += FloorStepX;
                FloorY += FloorStepY;
                
                u32 Colour = ((u32*)(GreyStone.Pixels))[TexelY*GreyStone.Width+TexelX];
                u8 R = Colour & 0xFF;
                u8 G = (Colour>>8) & 0xFF;
                u8 B = (Colour>>16) & 0xFF;
                u8 A = (Colour>>24) & 0xFF;
                
                Colour = (A<<24)|(R<<16)|(G<<8)|(B<<0);
                ((u32 *)(RState.Pixels))[ScreenY*RState.Width+ScreenX] = Colour;
            }
        }
        
        for (s32 ScreenX = 0; ScreenX < RState.Width; ++ScreenX)
        {
            f32 CameraX = ((2.0f*((f32)ScreenX / (f32)RState.Width)) - 1.0f)*(Right*W32State.AspectRatio);
            v2 RayDir = V2(PlayerDir.X + PlayerCameraDir.X * CameraX, 
                           PlayerDir.Y + PlayerCameraDir.Y * CameraX);
            
            f32 DeltaDistX = Math_Abs(1.0f / RayDir.X);
            f32 DeltaDistY = Math_Abs(1.0f / RayDir.Y);
            v2 OffsetWithinCell;
            s32 StepX, StepY, XSideHit = false;
            
            s32 RayXInt = (s32)PlayerP.X;
            s32 RayYInt = (s32)PlayerP.Y;
            
            if (RayDir.X < 0)
            {
                StepX = -1;
                OffsetWithinCell.X = (PlayerP.X - (f32)RayXInt) * DeltaDistX;
            }
            else
            {
                StepX = 1;
                OffsetWithinCell.X = ((f32)RayXInt + 1.0f - PlayerP.X) * DeltaDistX;
            }
            
            if (RayDir.Y < 0)
            {
                StepY = -1;
                OffsetWithinCell.Y = (PlayerP.Y - (f32)RayYInt) * DeltaDistY;
            }
            else
            {
                StepY = 1;
                OffsetWithinCell.Y = ((f32)RayYInt + 1.0f - PlayerP.Y) * DeltaDistY;
            }
            
            f32 DistanceToWall = 0;
            f32 MaxRaycastDepth = 16.0f;
            while (DistanceToWall < MaxRaycastDepth)
            {
                if (OffsetWithinCell.X < OffsetWithinCell.Y)
                {
                    DistanceToWall = OffsetWithinCell.X;
                    OffsetWithinCell.X += DeltaDistX;
                    RayXInt += StepX;
                    XSideHit = true;
                }
                else
                {
                    DistanceToWall = OffsetWithinCell.Y;
                    OffsetWithinCell.Y += DeltaDistY;
                    RayYInt += StepY;
                    XSideHit = false;
                }
                
                if (((RayXInt >= 0) && (RayXInt < 16)) && ((RayYInt >= 0) && (RayYInt < 16)) &&
                    (GameGrid[RayYInt * 16 + RayXInt] != 0))
                {
                    s32 CellValue = GameGrid[RayYInt * 16 + RayXInt] - 1;
                    f32 WallX;
                    if (XSideHit)
                    {
                        WallX = PlayerP.Y + DistanceToWall * RayDir.Y;
                    }
                    else
                    {
                        WallX = PlayerP.X + DistanceToWall * RayDir.X;
                    }
                    
                    WallX -= (f32)((s32)WallX);
                    w_assert((WallX>=0)&&(WallX<=1.0f));
                    s32 TexX = (s32)(WallX * (f32)Textures[CellValue].Width);
                    
                    if (XSideHit && (RayDir.X > 0)) TexX = Textures[CellValue].Width - TexX - 1;
                    if (!XSideHit && (RayDir.Y < 0)) TexX = Textures[CellValue].Width - TexX - 1;
                    
                    //u32 Colour = 0xffff0000;
                    //if (XSideHit) Colour &= 0x80808080;
                    
                    s32 WallHeight = (s32)((f32)RState.Height / DistanceToWall);
                    s32 DrawStartY = (RState.Height - WallHeight) / 2;
                    s32 DrawEndY = DrawStartY + WallHeight;
                    
                    R_VerticalLine(&RState, ScreenX, 0, DrawStartY, 0xff161616);
                    R_VerticalLineFromTexture2D(&RState, ScreenX, DrawStartY, DrawEndY, Textures[CellValue], TexX);
                    RState.DepthBuffer1D[ScreenX] = DistanceToWall;
                    break;
                }
            }
        }
        
        {
			R_Texture2D SpriteTex = Sprites[0];
            
            v2 SpriteWorldP = V2(3, 3);
            v2 SpriteCamP = V2(SpriteWorldP.X - PlayerP.X, SpriteWorldP.X - PlayerP.Y);
            f32 InvDet = 1.0f/(PlayerCameraDir.X*PlayerDir.Y-PlayerDir.X*PlayerCameraDir.Y);
            {
                // World To Camera
                f32 X = InvDet*(PlayerDir.Y*SpriteCamP.X - PlayerDir.X*SpriteCamP.Y);
                f32 Y = InvDet*(-PlayerCameraDir.Y*SpriteCamP.X + PlayerCameraDir.X*SpriteCamP.Y);
                SpriteCamP.X = X;
                SpriteCamP.Y = Y;
            }
            
            // Perspective Projection
            SpriteCamP.X /= SpriteCamP.Y;
            
            // NDC -> Screen Space
            s32 ScreenSpaceX = (s32)(((SpriteCamP.X+1)*0.5f)*RState.Width);
            s32 SpriteHeight = (s32)(Math_Abs((f32)RState.Height/SpriteCamP.Y));
            s32 SpriteWidth = SpriteHeight;
            s32 DrawStartY = (RState.Height - SpriteHeight) / 2;
			if (DrawStartY < 0)
			{
				DrawStartY = 0;
			}
            s32 DrawEndY = DrawStartY + SpriteHeight;
			if (DrawEndY >= RState.Height)
			{
				DrawEndY = RState.Height - 1;
			}
            s32 DrawStartX = ScreenSpaceX - SpriteWidth/2;
			if (DrawStartX < 0)
			{
				DrawStartX = 0;
			}
            s32 DrawEndX = ScreenSpaceX + SpriteWidth/2;
			if (DrawEndX >= RState.Width)
			{
				DrawEndX = RState.Width - 1;
			}
            
			for (s32 StripeX = DrawStartX; StripeX < DrawEndX; ++StripeX)
			{
				int TexelX = (s32)(256 * (StripeX - (-SpriteWidth/2 + ScreenSpaceX)) * SpriteTex.Width / SpriteWidth) / 256;
				if ((SpriteCamP.Y < RState.DepthBuffer1D[StripeX]) && (SpriteCamP.Y > 0) && (StripeX > 0) && (StripeX < RState.Width))
				{
                    for (s32 StripeY = DrawStartY; StripeY < DrawEndY; ++StripeY)
                    {
                        s32 D = StripeY*256 - RState.Height*128 + SpriteHeight*128;
                        s32 TexelY = (D*SpriteTex.Height/SpriteHeight)/256;
                        
                        u32 Colour = ((u32*)(SpriteTex.Pixels))[TexelY*SpriteTex.Width+TexelX];
                        u8 R = Colour & 0xFF;
                        u8 G = (Colour>>8) & 0xFF;
                        u8 B = (Colour>>16) & 0xFF;
                        u8 A = (Colour>>24) & 0xFF;
                        
                        if (R&&G&&B)
                        {
                            Colour = (A<<24)|(R<<16)|(G<<8)|(B<<0);
                            ((u32 *)(RState.Pixels))[StripeY*RState.Width+StripeX] = Colour;
                        }
                    }
				}
			}
        }
        
        u32 CellDim = 8;
        u32 Gap = 2;
        u32 InnerRectDim = 4;
        for (u32 YCell = 0; YCell < 16; ++YCell)
        {
            for (u32 XCell = 0; XCell < 16; ++XCell)
            {
                s32 CellValue = GameGrid[YCell * 16 + XCell];
                s32 X = XCell*CellDim + XCell*Gap;
                s32 Y = YCell*CellDim + YCell*Gap;
                
                R_WireRectangle(&RState, X, Y, CellDim, CellDim, 0xff0000ff);
                switch (CellValue)
                {
                    case 0: {} break;
                    case 1: case 2:
                    {
                        R_FillRectangle(&RState, X + InnerRectDim/2, Y + InnerRectDim/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xff0000ff);
                    } break;
                    
                    invalid_default_case();
                }
            }
        }
        
        {
            f32 XCell = PlayerP.X;
            f32 YCell = PlayerP.Y;
            s32 X = (s32)(XCell*CellDim + XCell*Gap);
            s32 Y = (s32)(YCell*CellDim + YCell*Gap);
            
            R_FillRectangle(&RState, X + InnerRectDim/2 - (CellDim - InnerRectDim)/2, Y + InnerRectDim/2 - (CellDim - InnerRectDim)/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xff00ff00);
        }
        
        HDC Hdc = GetDC(W32State.WindowHandle);
        
        BITMAPINFO BitmapInfo = {0};
        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biWidth = RState.Width;
        BitmapInfo.bmiHeader.biHeight = -(RState.Height);
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        
        StretchDIBits(Hdc, 0, 0, W32State.WindowWidth, W32State.WindowHeight,
                      0, 0, RState.Width, RState.Height,
                      RState.Pixels, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        
        //ZeroMemory(RState.Pixels, RState.Width*RState.Height*4);
        
        LARGE_INTEGER EndPerformanceCounter;
        QueryPerformanceCounter(&EndPerformanceCounter);
        
        u64 Difference = EndPerformanceCounter.QuadPart - BeginPerformanceCounter.QuadPart;
        u64 TimeElapsedMS = (Difference*1000)/PerformanceFrequency.QuadPart;
        if (TimeElapsedMS < MSPerFrame)
        {
            Sleep((DWORD)(MSPerFrame - TimeElapsedMS));
        }
        QueryPerformanceCounter(&BeginPerformanceCounter);
    }
    
    ExitProcess(0);
}