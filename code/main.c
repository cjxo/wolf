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
    fixed_16_16 GameUpdateS = f32_to_fixed_16_16(1.0f / (f32)MSPerFrame);
    
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
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    
    V2_Fixed PlayerP;
    fixed_16_16 PlayerDegreesRot = 0;
    fixed_16_16 RotRate = (50<<8)*(GameUpdateS>>8);
    PlayerP.X = (8 << 16);
    PlayerP.Y = (8 << 16);
    
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
            if ((PlayerDegreesRot>>16)<0)
            {
                PlayerDegreesRot = 360<<16;
            }
        }
        
        if (W32State.KeyFlags[INP_Key_Right] & INP_Flag_KeyHeld)
        {
            PlayerDegreesRot += RotRate;
            if ((PlayerDegreesRot>>16)>360)
            {
                PlayerDegreesRot = 0;
            }
        }
        
        fixed_16_16 RequestMoveX = 0;
        fixed_16_16 RequestMoveY = 0;
        
        if (W32State.KeyFlags[INP_Key_Up] & INP_Flag_KeyHeld)
        {
            V2_Fixed PlayerDir = {Math_Cos(PlayerDegreesRot>>16),Math_Sin(PlayerDegreesRot>>16)};
            
            RequestMoveX = fixed_16_16_mul(PlayerDir.X, 1<<16);
            RequestMoveX = fixed_16_16_mul(RequestMoveX, GameUpdateS);
            RequestMoveY = fixed_16_16_mul(PlayerDir.Y, 1<<16);
            RequestMoveY = fixed_16_16_mul(RequestMoveY, GameUpdateS);
        }
        
        if (W32State.KeyFlags[INP_Key_Down] & INP_Flag_KeyHeld)
        {
            V2_Fixed PlayerDir = {-Math_Cos(PlayerDegreesRot>>16),-Math_Sin(PlayerDegreesRot>>16)};
            
            RequestMoveX = fixed_16_16_mul(PlayerDir.X, 1<<16);
            RequestMoveX = fixed_16_16_mul(RequestMoveX, GameUpdateS);
            RequestMoveY = fixed_16_16_mul(PlayerDir.Y, 1<<16);
            RequestMoveY = fixed_16_16_mul(RequestMoveY, GameUpdateS);
        }
        
        if (RequestMoveY || RequestMoveX)
        {
            s32 OldXIDX = PlayerP.X >> 16;
            s32 OldYIDX = PlayerP.Y >> 16;
            
            fixed_16_16 X = PlayerP.X + RequestMoveX;
            fixed_16_16 Y = PlayerP.Y + RequestMoveY;
            
            s32 NewXIDX = X >> 16;
            s32 NewYIDX = Y >> 16;
            
            if ((NewXIDX < 16) && (NewYIDX < 16) && (NewXIDX >= 0) && (NewYIDX >= 0))
            {
                if (GameGrid[NewYIDX*16+NewXIDX] == 0)
                {
                    PlayerP.X = X;
                    PlayerP.Y = Y;
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
                    case 1: // Wall
                    {
                        R_FillRectangle(&RState, X + InnerRectDim/2, Y + InnerRectDim/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xff0000ff);
                    } break;
                    
                    invalid_default_case();
                }
            }
        }
        
        {
            s32 XCell = (PlayerP.X>>8);
            s32 YCell = (PlayerP.Y>>8);
            s32 X = (XCell*(CellDim<<8) + XCell*(Gap<<8))>>16;
            s32 Y = (YCell*(CellDim<<8) + YCell*(Gap<<8))>>16;
            
            R_FillRectangle(&RState, X + InnerRectDim/2, Y + InnerRectDim/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xff00ff00);
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