// NOTE(cj): TODOs
// - [X] Multiple Sprites
//    - [X] Sorting Sprites with respect to cam
// - [X] Random Dungeon Generation
//    - We will add roguelike elements here!
//    - [X] Basic BSP
//    - [X] Mini-Map improvements.
//       - Perhaps, Mini-Map is only enabled only on dev mode?
//       - [X] Scrolling
//       - [X] Raycasting Visualizer.
//    - [X] "Smooth Choppy" movement.
// - [X] Migrate to DCSS Tiles / Old TileSets
//    - [X] Just load em in some sort of list, for now.
// - [ ] Switch to D3D11 Renderer
//    - [ ] Try Creating an atlas from the DCSS tileset structure
//    - [ ] Try rendering DCSS in a 2D surface temporarily for testing purposes.
// - [ ] Generation improvements
//    - [ ] BSP + Cellular

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#include <shlwapi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./ext/stb_image.h"

#include "base.h"
#include "renderer.h"
#include "my_math.h"

#include "base.c"
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

typedef struct
{
    v2 P;
    R_Texture2D Texture;
} G_Sprite;

static G_Sprite
G_CreateSprite(v2 P, R_Texture2D Texture)
{
    G_Sprite Result;
    Result.Texture = Texture;
    Result.P = P;
    return(Result);
}

static s64
__G_SpriteOrderPartition(f32 *SpriteLengthSquareds, u64 *ResultSortOrder, s64 LowSide, s64 HighSide)
{
    f32 Pivot = SpriteLengthSquareds[HighSide];
    s64 HighestLowSide = LowSide;
    
    for (s64 ArrayIndex = LowSide; ArrayIndex < HighSide; ++ArrayIndex)
    {
        if (SpriteLengthSquareds[ArrayIndex] >= Pivot)
        {
            swap(SpriteLengthSquareds[ArrayIndex], SpriteLengthSquareds[HighestLowSide], f32);
            swap(ResultSortOrder[ArrayIndex], ResultSortOrder[HighestLowSide], u64);
            HighestLowSide += 1;
        }
    }
    
    swap(SpriteLengthSquareds[HighSide], SpriteLengthSquareds[HighestLowSide], f32);
    swap(ResultSortOrder[HighSide], ResultSortOrder[HighestLowSide], u64);
    return(HighestLowSide);
}

static void
__G_SpriteOrder(f32 *SpriteLengthSquareds, u64 *ResultSortOrder, s64 LowSide, s64 HighSide)
{
    if (LowSide < HighSide)
    {
        s64 PartitionIndex = __G_SpriteOrderPartition(SpriteLengthSquareds, ResultSortOrder, LowSide, HighSide);
        __G_SpriteOrder(SpriteLengthSquareds, ResultSortOrder, LowSide, PartitionIndex - 1);
        __G_SpriteOrder(SpriteLengthSquareds, ResultSortOrder, PartitionIndex + 1, HighSide);
    }
}

static void
G_SpriteOrder(f32 *SpriteLengthSquareds, u64 SpriteCount, u64 *ResultSortOrder)
{
    // NOTE: ResultSortOrder Length must be equal to SpriteCount.
    for (u64 SpriteIdx = 0; SpriteIdx < SpriteCount; ++SpriteIdx)
    {
        ResultSortOrder[SpriteIdx] = SpriteIdx;
    }
    
    __G_SpriteOrder(SpriteLengthSquareds, ResultSortOrder, 0, SpriteCount - 1);
}

typedef struct
{
    s32 ID;
} G_Tile;

typedef u32 G_AssetType;
enum
{
    G_AssetType_Folder,
    G_AssetType_Texture2D,
    G_AssetType_Count,
};
typedef struct G_Asset G_Asset;
struct G_Asset
{
    G_AssetType Type;
    String_U8_Const Name;
    R_Texture2D Texture;
    G_Asset *NextSibling, *PrevSibling;
    G_Asset *FirstChild, *LastChild;
};

inline static G_Asset *
G_CreateAsset(M_Arena *Arena, G_AssetType Type, String_U8_Const Name)
{
    G_Asset *Result = m_arena_push(Arena, G_Asset);
    Result->Type = Type;
    Result->Name = Str8_Copy(Name, Arena);
    return(Result);
}

static void
G_LoadDCSSTiles(G_Asset **RootAsset, M_Arena *Arena, char *FolderName)
{
    HANDLE SearchHandle;
    WIN32_FIND_DATAA FindData;
    char Buffer[MAX_PATH];
    
    PathCombineA(Buffer, FolderName, "*");
    SearchHandle = FindFirstFileA(Buffer, &FindData);
    w_assert(SearchHandle != INVALID_HANDLE_VALUE);
    
    do
    {
        if ((FindData.cFileName[0] != '.') && (FindData.cFileName[1] != '.'))
        {
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                PathCombineA(Buffer, FolderName, FindData.cFileName);
                G_Asset *Asset = G_CreateAsset(Arena, G_AssetType_Folder, Str8_FromCSTR(FindData.cFileName));
                
                dll_push_back_np((*RootAsset)->FirstChild,
                                 (*RootAsset)->LastChild,
                                 Asset,
                                 NextSibling,
                                 PrevSibling);
                
                G_LoadDCSSTiles(&Asset, Arena, Buffer);
            }
            else
            {
                G_Asset *Asset = G_CreateAsset(Arena, G_AssetType_Texture2D, Str8_FromCSTR(FindData.cFileName));
                
                dll_push_back_np((*RootAsset)->FirstChild,
                                 (*RootAsset)->LastChild,
                                 Asset,
                                 NextSibling,
                                 PrevSibling);
            }
        }
    } while (FindNextFileA(SearchHandle, &FindData));
    
    FindClose(SearchHandle);
}

static void
G_Debug_PrintDCSSTiles(G_Asset *Root, u32 Depth)
{
    char Buffer[256];
    for (G_Asset *Asset = Root->FirstChild;
         Asset;
         Asset = Asset->NextSibling)
    {
        switch (Asset->Type)
        {
            case G_AssetType_Folder:
            {
                wsprintfA(Buffer, "[[%s]]\n", Asset->Name.S);
                for (u32 DepthIdx = 0; DepthIdx < Depth; ++DepthIdx)
                {
                    OutputDebugStringA(" ");
                }
                OutputDebugStringA(Buffer);
                G_Debug_PrintDCSSTiles(Asset, Depth + 1);
                for (u32 DepthIdx = 0; DepthIdx < Depth; ++DepthIdx)
                {
                    OutputDebugStringA(" ");
                }
                OutputDebugStringA(Buffer);
            } break;
            
            case G_AssetType_Texture2D:
            {
                wsprintfA(Buffer, "[File]: %s\n", Asset->Name.S);
                for (u32 DepthIdx = 0; DepthIdx < Depth; ++DepthIdx)
                {
                    OutputDebugStringA(" ");
                }
                OutputDebugStringA(Buffer);
            } break;
            
            invalid_default_case();
        }
    }
}

static void
__G_GEN_BSP(PRNG *Gen, G_Tile *Tiles, s32 TileMapWidth, s32 TileMapHeight, s32 StartGenX, s32 StartGenY, s32 EndGenX, s32 EndGenY, s32 MinRoomW, s32 MinRoomH)
{
    s32 RoomWidthS32 = EndGenX-StartGenX;
    s32 RoomHeightS32 = EndGenY-StartGenY;
    if ((RoomHeightS32 <= MinRoomH) || (RoomWidthS32 <= MinRoomW))
    {
        s32 DesiredPosX = StartGenX;
        s32 DesiredPosY = StartGenY;
        
        if (DesiredPosX == 0)
        {
            DesiredPosX = 1;
        }
        
        if (DesiredPosY == 0)
        {
            DesiredPosY = 1;
        }
        
        //s32 DesiredEndPosX = DesiredPosX + PRNG_RangeU32(Gen, ((EndGenX - StartGenX) * 3) / 4, EndGenX - StartGenX);
        s32 DesiredEndPosX = EndGenX;
        //s32 DesiredEndPosY = DesiredPosY + PRNG_RangeU32(Gen, ((EndGenY - StartGenY) * 3) / 4, EndGenY - StartGenY);
        s32 DesiredEndPosY = EndGenY;
        
        if (DesiredEndPosX == TileMapWidth)
        {
            DesiredEndPosX -= 1;
        }
        
        if (DesiredEndPosY == TileMapHeight)
        {
            DesiredEndPosY -= 1;
        }
        
        while (DesiredPosY < DesiredEndPosY)
        {
            for (s32 CurrX = DesiredPosX; CurrX < DesiredEndPosX; ++CurrX)
            {
                Tiles[DesiredPosY * TileMapWidth + CurrX].ID = 0;
            }
            ++DesiredPosY;
        }
    }
    else
    {
        u32 AntiInfLoop = 0;
        static f32 MaxDimsRatio = 2.3f;
        forever
        {
            b32 SplitHorizontal = PRNG_NormF32(Gen) < 0.5f;
            if (SplitHorizontal)
            {
                // ------------------
                // -                -
                // -                -
                // -                -
                // ------------------    <-------- DesiredPosY
                // -                -
                // -                -
                // -                -
                // ------------------
                u32 DesiredPosY = PRNG_RangeU32(Gen, StartGenY, EndGenY);
                
                f32 RoomWidth = (f32)RoomWidthS32;
                f32 TopRoomHeight = (f32)(DesiredPosY - StartGenY);
                f32 BottomRoomHeight = (f32)(EndGenY - DesiredPosY);
                
                f32 TopRoomRatio = RoomWidth / TopRoomHeight;
                f32 BottomRoomRatio = RoomWidth / BottomRoomHeight;
                
                if (!((TopRoomRatio > MaxDimsRatio) || (BottomRoomRatio > MaxDimsRatio)))
                {
                    __G_GEN_BSP(Gen, Tiles, TileMapWidth, TileMapHeight, StartGenX, StartGenY, EndGenX, DesiredPosY, MinRoomW, MinRoomH);
                    __G_GEN_BSP(Gen, Tiles, TileMapWidth, TileMapHeight, StartGenX, DesiredPosY + 1, EndGenX, EndGenY, MinRoomW, MinRoomH);
                    
                    s32 StartHoleX = RoomWidthS32/2 + StartGenX;
                    s32 StartHoleY = DesiredPosY - (DesiredPosY - StartGenY)/2;
                    s32 EndHoleY = (EndGenY - DesiredPosY)/2 + DesiredPosY;
                    
                    for (; StartHoleY < EndHoleY; ++StartHoleY)
                    {
                        Tiles[StartHoleY * TileMapWidth + StartHoleX].ID = 0;
                    }
                    break;
                }
            }
            else
            {
                // ------------------
                // -       -        -
                // -       -        -
                // -       -        -
                // -       -        -
                // -       -        -
                // -       -        -
                // -       -        -
                // ------------------
                //         ^
                //         |
                //         |
                //    DesiredPosX
                u32 DesiredPosX = PRNG_RangeU32(Gen, StartGenX, EndGenX);
                
                f32 RoomHeight = (f32)RoomHeightS32;
                f32 LeftRoomWidth = (f32)(DesiredPosX - StartGenX);
                f32 RightRoomWidth = (f32)(EndGenX - DesiredPosX);
                
                f32 LeftRoomRatio = RoomHeight / LeftRoomWidth;
                f32 RightRoomRatio = RoomHeight / RightRoomWidth;
                
                if (!((LeftRoomRatio > MaxDimsRatio) || (RightRoomRatio > MaxDimsRatio)))
                {
                    __G_GEN_BSP(Gen, Tiles, TileMapWidth, TileMapHeight, StartGenX, StartGenY, DesiredPosX, EndGenY, MinRoomW, MinRoomH);
                    __G_GEN_BSP(Gen, Tiles, TileMapWidth, TileMapHeight, DesiredPosX + 1, StartGenY, EndGenX, EndGenY, MinRoomW, MinRoomH);
                    
                    s32 StartHoleY = StartGenY + RoomHeightS32/2;
                    s32 StartHoleX = DesiredPosX - (DesiredPosX - StartGenX)/2;
                    s32 EndHoleX = (EndGenX - DesiredPosX)/2 + DesiredPosX;
                    for (; StartHoleX < EndHoleX; ++StartHoleX)
                    {
                        Tiles[StartHoleY * TileMapWidth + StartHoleX].ID = 0;
                    }
                    break;
                }
            }
            
            if (++AntiInfLoop > 100)
            {
                break;
            }
        }
    }
}

inline static void
G_GEN_BSP(PRNG *Gen, G_Tile *Tiles, u32 Width, u32 Height, s32 MinRoomW, s32 MinRoomH)
{
    u32 Area = Width*Height;
    for (u32 Index = 0;
         Index < Area;
         ++Index)
    {
        Tiles[Index].ID = 1;
    }
    
    __G_GEN_BSP(Gen, Tiles, Width, Height, 0, 0, Width, Height, MinRoomW, MinRoomH);
}

#define G_MaxMapDims 32
s32 __stdcall
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, s32 nShowCmd)
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
    
    G_Tile GameGrid[G_MaxMapDims*G_MaxMapDims];
    v2 PlayerP = V2(8.5f, 8.5f), NextPlayerP;
    f32 PlayerMoveT = -1.0f;
    
    f32 PlayerDegreesRot = 0, NextPlayerDegreesRot, PlayerDegreesRotT = -1.0f;
    f32 RotRate = 40.0f*GameUpdateS;
    
    R_Texture2D GreyStone;
    R_LoadTexture(&GreyStone, "..\\data\\textures\\greystone.png");
    
    R_Texture2D SpriteTextures[2];
    R_LoadTexture(SpriteTextures + 0, "..\\data\\textures\\barrel.png");
    R_LoadTexture(SpriteTextures + 1, "..\\data\\textures\\pillar.png");
    
    R_Texture2D Textures[2];
    R_LoadTexture(Textures + 0, "..\\data\\textures\\eagle.png");
    R_LoadTexture(Textures + 1, "..\\data\\textures\\redbrick.png");
    
    G_Sprite StaticSprites[] =
    {
        G_CreateSprite(V2(6.5f, 3.5f), SpriteTextures[1]),
        G_CreateSprite(V2(3.5f, 3.5f), SpriteTextures[0]),
    };
    
    f32 Fov = 66.0f * (3.14159f/180.0f);
    f32 Right = tanf(Fov*0.5f);
    f32 MaxRaycastDepth = 16.0f;
    s32 MinWallHeight = (s32)(Right*MaxRaycastDepth);
    
    PRNG RandGen;
    PRNG_Seed(&RandGen, 37);
    G_GEN_BSP(&RandGen, GameGrid, G_MaxMapDims, G_MaxMapDims, 10, 10);
    
    M_Arena *Arena = M_ArenaReserve(gb(1));
    G_Asset *DCSSAssetsRootFolder = G_CreateAsset(Arena, G_AssetType_Folder, str8("crawl-tiles"));
    G_LoadDCSSTiles(&DCSSAssetsRootFolder, Arena, "../data/textures/crawl-tiles");
    
    G_Debug_PrintDCSSTiles(DCSSAssetsRootFolder, 0);
    
    //ExitProcess(0);
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
        
        if (PlayerDegreesRotT != -1.0f)
        {
            if (PlayerDegreesRotT >= 1.0f)
            {
                PlayerDegreesRotT = -1.0f;
                PlayerDegreesRot = NextPlayerDegreesRot;
            }
            else
            {
                PlayerDegreesRotT += GameUpdateS;
                f32 T = 1.0f - PlayerDegreesRotT;
                PlayerDegreesRot = PlayerDegreesRot + (1.0f-T*T)*(NextPlayerDegreesRot - PlayerDegreesRot);
            }
        }
        else if (W32State.KeyFlags[INP_Key_Left] & INP_Flag_KeyHeld)
        {
            PlayerDegreesRotT = 0.0f;
            NextPlayerDegreesRot = PlayerDegreesRot - 45;
            if (NextPlayerDegreesRot < 0)
            {
                PlayerDegreesRot = 360;
                NextPlayerDegreesRot = 315;
            }
        }
        else if (W32State.KeyFlags[INP_Key_Right] & INP_Flag_KeyHeld)
        {
            PlayerDegreesRotT = 0.0f;
            NextPlayerDegreesRot = PlayerDegreesRot + 45;
            if (NextPlayerDegreesRot > 360)
            {
                PlayerDegreesRot = 0;
                NextPlayerDegreesRot = 45;
            }
        }
        
        f32 RequestMoveX = 0;
        f32 RequestMoveY = 0;
        
        // NOTE(cj): These vectors are in world coordinate system
        // These two vectors can be made into a matrix that transforms the camera to world space.
        v2 PlayerDir = V2(Math_Cos((u32)PlayerDegreesRot),Math_Sin((u32)PlayerDegreesRot));
        v2 PlayerCameraDir = V2(-PlayerDir.Y*Right, PlayerDir.X*Right);
        
        if (PlayerMoveT != -1.0f)
        {
            if (PlayerMoveT >= 1.0f)
            {
                PlayerMoveT = -1.0f;
                PlayerP = NextPlayerP;
            }
            else
            {
                f32 T = 1.0f - PlayerMoveT;
                T = 1.0f - T*T;
                PlayerP.X = PlayerP.X + T*(NextPlayerP.X-PlayerP.X);
                PlayerP.Y = PlayerP.Y + T*(NextPlayerP.Y-PlayerP.Y);
                PlayerMoveT += GameUpdateS*2;
            }
        }
        
        if ((PlayerMoveT == -1.0f) && (W32State.KeyFlags[INP_Key_Up] & INP_Flag_KeyHeld))
        {
            RequestMoveX = PlayerDir.X * 1;
            RequestMoveY = PlayerDir.Y * 1;
        }
        
        if ((PlayerMoveT == -1.0f) && (W32State.KeyFlags[INP_Key_Q] & INP_Flag_KeyHeld))
        {
            RequestMoveX = PlayerDir.Y * 1;
            RequestMoveY = -PlayerDir.X * 1;
        }
        
        if ((PlayerMoveT == -1.0f) && (W32State.KeyFlags[INP_Key_E] & INP_Flag_KeyHeld))
        {
            RequestMoveX = -PlayerDir.Y * 1;
            RequestMoveY = PlayerDir.X * 1;
        }
        
        if ((PlayerMoveT == -1.0f) && (W32State.KeyFlags[INP_Key_Down] & INP_Flag_KeyHeld))
        {
            RequestMoveX = -PlayerDir.X * 1;
            RequestMoveY = -PlayerDir.Y * 1;
        }
        
        if (RequestMoveY || RequestMoveX)
        {
            if (RequestMoveX > 0)
            {
                RequestMoveX = (f32)(s32)(RequestMoveX + 0.5f);
            }
            else
            {
                RequestMoveX = (f32)(s32)(RequestMoveX - 0.5f);
            }
            
            if (RequestMoveY > 0)
            {
                RequestMoveY = (f32)(s32)(RequestMoveY + 0.5f);
            }
            else
            {
                RequestMoveY= (f32)(s32)(RequestMoveY - 0.5f);
            }
            
            NextPlayerP = V2(PlayerP.X + RequestMoveX, PlayerP.Y + RequestMoveY);
            
            s32 NewXIDX = (s32)NextPlayerP.X;
            s32 NewYIDX = (s32)NextPlayerP.Y;
            
            if ((NewXIDX < G_MaxMapDims) && (NewYIDX < G_MaxMapDims) && (NewXIDX >= 0) && (NewYIDX >= 0))
            {
                if (GameGrid[NewYIDX*G_MaxMapDims+NewXIDX].ID == 0)
                {
                    PlayerMoveT = 0.0f;
                }
            }
        }
        
        // ------ THE ACTUAL MAIN RENDERER ------ //
        v2 RayDir0 = V2(PlayerDir.X - PlayerCameraDir.X, PlayerDir.Y - PlayerCameraDir.Y);
        v2 RayDir1 = V2(PlayerDir.X + PlayerCameraDir.X, PlayerDir.Y + PlayerCameraDir.Y);
        f32 PosZ = RState.Height*0.5f;
        for (s32 ScreenY = RState.Height/2; ScreenY < RState.Height; ++ScreenY)
        {
            s32 YPFromCenter = ScreenY - RState.Height/2;
            if (YPFromCenter >= MinWallHeight)
            {
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
                    
                    if ((TexelX >= 0) && (TexelY >= 0))
                    {
                        u32 Colour = ((u32*)(GreyStone.Pixels))[TexelY*GreyStone.Width+TexelX];
                        u8 R = Colour & 0xFF;
                        u8 G = (Colour>>8) & 0xFF;
                        u8 B = (Colour>>16) & 0xFF;
                        u8 A = (Colour>>24) & 0xFF;
                        
                        Colour = (A<<24)|(R<<16)|(G<<8)|(B<<0);
                        ((u32 *)(RState.Pixels))[ScreenY*RState.Width+ScreenX] = Colour;
                    }
                }
            }
        }
        
        u8 MinimapHitState[G_MaxMapDims*G_MaxMapDims] = {0};
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
                
                if (((RayXInt >= 0) && (RayXInt < G_MaxMapDims)) && ((RayYInt >= 0) && (RayYInt < G_MaxMapDims)))
                {
                    MinimapHitState[RayYInt * G_MaxMapDims + RayXInt] = 1;
                    if (GameGrid[RayYInt * G_MaxMapDims + RayXInt].ID != 0)
                    {
                        s32 CellValue = GameGrid[RayYInt * G_MaxMapDims + RayXInt].ID - 1;
                        //MinimapHitState[RayYInt * G_MaxMapDims + RayXInt] = 1;
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
                        
                        s32 WallHeight = (s32)((f32)RState.Height / DistanceToWall);
                        s32 DrawStartY = (RState.Height - WallHeight) / 2;
                        s32 DrawEndY = DrawStartY + WallHeight;
                        
                        R_VerticalLine(&RState, ScreenX, 0, DrawStartY, 0);
                        R_VerticalLineFromTexture2D(&RState, ScreenX, DrawStartY, DrawEndY, Textures[CellValue], TexX);
                        RState.DepthBuffer1D[ScreenX] = DistanceToWall;
                        break;
                    }
                }
            }
        }
        
        f32 SpriteLengthSquareds[array_count(StaticSprites)];
        for (u32 SpriteIdx = 0; SpriteIdx < array_count(StaticSprites); ++SpriteIdx)
        {
            f32 X = StaticSprites[SpriteIdx].P.X - PlayerP.X;
            f32 Y = StaticSprites[SpriteIdx].P.Y - PlayerP.Y;
            SpriteLengthSquareds[SpriteIdx] = X*X + Y*Y;
        }
        u64 SpriteOrderRender[array_count(StaticSprites)];
        G_SpriteOrder(SpriteLengthSquareds, array_count(StaticSprites), SpriteOrderRender);
        
        for (u32 SpriteIdx = 0; SpriteIdx < array_count(StaticSprites); ++SpriteIdx)
        {
            G_Sprite Sprite = StaticSprites[SpriteOrderRender[SpriteIdx]];
            
            v2 SpriteWorldP = Sprite.P;
            v2 SpriteCamP = V2(SpriteWorldP.X - PlayerP.X, SpriteWorldP.X - PlayerP.Y);
            f32 InvDet = 1.0f/(PlayerCameraDir.X*PlayerDir.Y-PlayerDir.X*PlayerCameraDir.Y);
            
            // World To Camera
            f32 X = InvDet*(PlayerDir.Y*SpriteCamP.X - PlayerDir.X*SpriteCamP.Y);
            f32 Y = InvDet*(-PlayerCameraDir.Y*SpriteCamP.X + PlayerCameraDir.X*SpriteCamP.Y);
            SpriteCamP.X = X;
            SpriteCamP.Y = Y;
            
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
				int TexelX = (s32)(256 * (StripeX - (-SpriteWidth/2 + ScreenSpaceX)) * Sprite.Texture.Width / SpriteWidth) / 256;
				if ((SpriteCamP.Y < RState.DepthBuffer1D[StripeX]) && (SpriteCamP.Y > 0) && (StripeX > 0) && (StripeX < RState.Width))
				{
                    for (s32 StripeY = DrawStartY; StripeY < DrawEndY; ++StripeY)
                    {
                        s32 D = StripeY*256 - RState.Height*128 + SpriteHeight*128;
                        s32 TexelY = (D*Sprite.Texture.Height/SpriteHeight)/256;
                        
                        u32 Colour = ((u32*)(Sprite.Texture.Pixels))[TexelY*Sprite.Texture.Width+TexelX];
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
        
        s32 CamDims = 16;
        s32 MinimapCamOffsetX = (s32)PlayerP.X;
        if (MinimapCamOffsetX < CamDims/2)
        {
            MinimapCamOffsetX = CamDims/2;
        }
        else if ((MinimapCamOffsetX + CamDims/2) > G_MaxMapDims)
        {
            MinimapCamOffsetX = G_MaxMapDims - CamDims/2;
        }
        
        s32 MinimapCamOffsetY = (s32)PlayerP.Y;
        if (MinimapCamOffsetY < CamDims/2)
        {
            MinimapCamOffsetY = CamDims/2;
        }
        else if ((MinimapCamOffsetY + CamDims/2) > G_MaxMapDims)
        {
            MinimapCamOffsetY = G_MaxMapDims - CamDims/2;
        }
        u32 CellDim = 8;
        u32 Gap = 2;
        u32 InnerRectDim = 4;
        
        // world space...
        s32 StartIterY = MinimapCamOffsetY - CamDims/2;
        s32 EndIterY = CamDims + StartIterY;
        s32 StartIterX = MinimapCamOffsetX - CamDims/2;
        s32 EndIterX = CamDims + StartIterX;
        
        for (s32 YCell = StartIterY; YCell < EndIterY; ++YCell)
        {
            for (s32 XCell = StartIterX; XCell < EndIterX; ++XCell)
            {
                s32 CellValue = GameGrid[YCell * G_MaxMapDims + XCell].ID;
                // camera (minimap) space...
                s32 X = (XCell - MinimapCamOffsetX + CamDims/2)*(CellDim + Gap);
                s32 Y = (YCell - MinimapCamOffsetY + CamDims/2)*(CellDim + Gap);
                
                if (MinimapHitState[YCell*G_MaxMapDims+XCell])
                {
                    R_WireRectangle(&RState, X, Y, CellDim, CellDim, 0xffffffff);
                    R_FillRectangle(&RState, X + InnerRectDim/2, Y + InnerRectDim/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xffffffff);
                }
                else
                {
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
        }
        
        {
            s32 X = (((s32)PlayerP.X - MinimapCamOffsetX + CamDims/2)*(CellDim + Gap));
            s32 Y = (((s32)PlayerP.Y - MinimapCamOffsetY + CamDims/2)*(CellDim + Gap));
            
            R_FillRectangle(&RState, X + InnerRectDim - (CellDim - InnerRectDim)/2, Y + InnerRectDim - (CellDim - InnerRectDim)/2, CellDim - InnerRectDim, CellDim - InnerRectDim, 0xff00ff00);
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
        
        ZeroMemory(RState.Pixels, RState.Width*RState.Height*4);
        //ZeroMemory(RState.DepthBuffer1D, RState.Width*RState.Height*4);
        
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