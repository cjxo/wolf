#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>

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
    INP_Flags KeyFlags[INP_Key_Count];
} W32_State;

static LRESULT __stdcall
W32_WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    
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
    HWND WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "Wolfenstein", WS_OVERLAPPEDWINDOW, 0, 0, WindowWidth, WindowHeight, 0, 0, WindowClass.hInstance, 0);
    
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

void __stdcall
EntryPoint(void)
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
        
        v2 PlayerDir = V2(Math_Cos((u32)PlayerDegreesRot),Math_Sin((u32)PlayerDegreesRot));
        v2 PlayerCameraDir = V2(-PlayerDir.Y*0.66f, PlayerDir.X*0.66f);
        
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
        
        for (s32 ScreenX = 0; ScreenX < RState.Width; ++ScreenX)
        {
            f32 CameraX = (2.0f*((f32)ScreenX / (f32)RState.Width)) - 1.0f;
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
                    s32 CellValue = GameGrid[RayYInt * 16 + RayXInt];
                    u32 Colour = 0;
                    switch (CellValue)
                    {
                        case 1: Colour = 0xffff0000; break;
                        case 2: Colour = 0xff00ff00; break;
                        invalid_default_case();
                    }
                    
                    if (XSideHit) Colour &= 0x80808080;
                    
                    s32 WallHeight = (s32)((f32)RState.Height / DistanceToWall);
                    s32 DrawStartY = (RState.Height - WallHeight) / 2;
                    s32 DrawEndY = DrawStartY + WallHeight;
                    
                    R_VerticalLine(&RState, ScreenX, DrawStartY, DrawEndY, Colour);
                    break;
                }
            }
        }
        
        u32 CellDim = 16;
        u32 Gap = 4;
        u32 InnerRectDim = 8;
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
        
        ZeroMemory(RState.Pixels, 1280*720*4);
        
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