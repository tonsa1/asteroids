#include "asteroids_platform.h"

#include <windows.h>
#include <stdio.h>

#include "win32_asteroids.h"

global_variable b32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable s64 GlobalPerfCountFrequency;

internal void
DEBUGPlatformFreeFileMemory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
        }
    }
    
    return Result;
}

internal FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFileA(Filename, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE)
    {
        LastWriteTime = FindData.ftLastWriteTime;
        FindClose(FindHandle);
    }
    
    return LastWriteTime;
}

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName)
{
    win32_game_code Result = {};
    
    char *TempDLLName = "asteroids_temp.dll";
    //char *TempDLLName = "asteroids.dll";
    Result.DLLLastWriteTime = Win32GetLastWriteTime(TempDLLName);
    
    CopyFile(SourceDLLName,TempDLLName, FALSE);
    Result.GameCodeDLL = LoadLibraryA(TempDLLName);
    if(Result.GameCodeDLL)
    {
        Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
        
        Result.IsValid = Result.UpdateAndRender && 1;
    }
    
    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
    }
    
    return Result;
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
    }
    
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return Result;
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = (Width*Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
                           int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer *Buffer)
{
    int OffsetX = 10;
    int OffsetY = 10;
    
    PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
    PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);
    
#if 1
    StretchDIBits(DeviceContext,
                  OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory, &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
#endif
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SIZE:
        {
        } break;
        
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
        } break;
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, 1280, 720, &GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            //            OutputDebugString("Default");
            Result =  DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

internal void
ProcessKeyboardInput(game_button_state *Key, b32 IsDown)
{
    Key->EndedDown = IsDown;
    ++Key->HalfTransitionCount;
}

void
HandleDebugCounters(game_memory *Memory)
{
    for(u64 Index = 0;
        Index <
        ArrayCount(Memory->Counters);
        ++Index)
    {
        cycle_count *Counter = Memory->Counters + Index;
        char TextBuffer[256];
        if(Counter->Hits != 0)
        {
            
            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                        "%I64u Cycles: %I64u Hits: %I64u cyclesPerHit: %I64u\n\n",
                        Index, Counter->Cycles, Counter->Hits,
                        (u64)((Counter->Cycles / Counter->Hits) / 8));
            OutputDebugStringA(TextBuffer);
            
            Counter->Cycles = 0;
            Counter->Hits = 0;
        }
        
    }
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) /
                  (f32)GlobalPerfCountFrequency);
    return(Result);
}

struct platform_platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    u32 volatile EntryTop;
    u32 volatile EntryBottom;
    HANDLE SemaphoreHandle;
    
    platform_platform_work_queue_entry Entries[32];
};

void 
PlatformAddWorkQueueEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    Assert(Queue->EntryTop < ArrayCount(Queue->Entries));
    
    u32 NewEntryTop = (Queue->EntryTop + 1) % ArrayCount(Queue->Entries);
    platform_platform_work_queue_entry *Entry = Queue->Entries + Queue->EntryTop;
    Entry->Data = Data;
    Entry->Callback = Callback;
    
    _WriteBarrier();
    
    Queue->EntryTop = NewEntryTop;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

b32
PlatformDoNextWorkQueueEntry(platform_work_queue *Queue, u32 LogicalThreadIndex)
{
    b32 ShouldWeSleep = false;
    
    u32 OriginalEntryBottom = Queue->EntryBottom;
    u32 NewEntryBottom = (OriginalEntryBottom + 1) % ArrayCount(Queue->Entries);
    if(OriginalEntryBottom != Queue->EntryTop)
    {
        u32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->EntryBottom,
                                               NewEntryBottom,
                                               OriginalEntryBottom);
        if(Index == OriginalEntryBottom)
        {
            platform_platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Entry.Data, LogicalThreadIndex);
        }
    }
    else
    {
        ShouldWeSleep = true;
    }
    
    return ShouldWeSleep;
}

void
PlatformCompleteAllQueueWork(platform_work_queue *Queue, u32 LogicalThreadIndex)
{
    while(!PlatformDoNextWorkQueueEntry(Queue, LogicalThreadIndex))
    {
        
    }
}

inline 
PLATFORM_WORK_QUEUE_CALLBACK(DoStringWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", LogicalThreadIndex, (char *)Data);
    OutputDebugString(Buffer);
}

internal void
PrintString(char* String, u32 Thread)
{
    char Buffer[256];
    wsprintf(Buffer, "%u %s", Thread, String);
    OutputDebugString(Buffer);
}

struct win32_thread_info
{
    int LogicalThreadIndex;
    platform_work_queue *Queue;
};

DWORD WINAPI 
ThreadProc(LPVOID lpParameter)
{
    win32_thread_info *ThreadInfo = (win32_thread_info *)lpParameter;
    for(;;)
    {
        if(PlatformDoNextWorkQueueEntry(ThreadInfo->Queue, ThreadInfo->LogicalThreadIndex))
        {
            WaitForSingleObjectEx(ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    } 
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PreviousInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    win32_thread_info ThreadInfo[10];
    u32 ThreadCount = ArrayCount(ThreadInfo);
    
    platform_work_queue Queue = {};
    
    u32 InitialCount = 0;
    Queue.SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    
    for(u32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = &Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    s64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    UINT DesiredschedulerMS = 1.0f;
    b32 IsGranular = timeBeginPeriod(DesiredschedulerMS) == TIMERR_NOERROR;
    
    WNDCLASSA WindowClass = {};
    
    int ScreenDimensionX = 720;
    int ScreenDimensionY = 480;
    
    Win32ResizeDIBSection(&GlobalBackbuffer, ScreenDimensionX, ScreenDimensionY);
    
    WindowClass.style = CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon
    WindowClass.lpszClassName = "Handmade Asteroids";
    
    f32 TargetFramesPerSecond = 60.0f;
    f32 TargetMS = 1000.0f*(1.0f / 60.0f);
    b32 LockedFPS = false;
    f32 InverseMS = 1.0f / 1000.0f;
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(0,
                            WindowClass.lpszClassName,
                            "Handmade Asteroids",
                            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            Instance,
                            0);
        
        
        if(Window)
        {
            
            game_controller_input KeyboardInput = {};
            game_input GameInput = {};
            GameInput.dtForFrame = TargetMS * InverseMS;
            GameInput.KeyboardInput = &KeyboardInput;
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(3);
            GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize,
                                                       MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorageSize = Megabytes(2);
            GameMemory.TransientStorage = (void *)((char *)GameMemory.PermanentStorage +
                                                   GameMemory.TransientStorageSize);
            
            GameMemory.DEBUGPlatformFreeFileMemory = &DEBUGPlatformFreeFileMemory;
            GameMemory.DEBUGPlatformReadEntireFile = &DEBUGPlatformReadEntireFile;
            GameMemory.ScreenDimensionY = ScreenDimensionY;
            GameMemory.ScreenDimensionX = ScreenDimensionX;
            GameMemory.PlatformCompleteAllWorkQueueWork = PlatformCompleteAllQueueWork;
            GameMemory.PlatformAddWorkQueueEntry = PlatformAddWorkQueueEntry;
            GameMemory.PlatformDoNextWorkQueueEntry = PlatformDoNextWorkQueueEntry;
            GameMemory.DoStringWork = DoStringWork;
            GameMemory.Queue = &Queue;
            GameMemory.PrintString = PrintString;
            char *SourceDLLName = "asteroids.dll";
            win32_game_code Game = Win32LoadGameCode(SourceDLLName);
            
            GlobalRunning = true;
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            s64 LastCycleCount = __rdtsc();
            f32 SleepTime = 0;
            while(GlobalRunning)
            {
                LARGE_INTEGER WorkCounter = Win32GetWallClock();
                f32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                
                if(LockedFPS)
                {
                    
                    if(IsGranular)
                    {
                        if(SleepTime > 0.0f)
                        {
                            Sleep(SleepTime);
                        }
                    }
                }
                
                FILETIME NewWriteTime = Win32GetLastWriteTime(SourceDLLName);
                if(CompareFileTime(&NewWriteTime, &Game.DLLLastWriteTime) != 0)
                {
                    Win32UnloadGameCode(&Game);
                    Game = Win32LoadGameCode(SourceDLLName);
                }
                
                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    switch (Message.message)
                    {
                        case WM_QUIT:
                        {
                            GlobalRunning = false;
                        } break;
                        
                        case WM_SYSKEYDOWN:
                        case WM_SYSKEYUP:
                        case WM_KEYDOWN:
                        case WM_KEYUP:
                        {
                            u32 VKCode = (u32)Message.wParam;
                            b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                            b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                            if(WasDown != IsDown)
                            {
                                if(VKCode == VK_RIGHT)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Right, IsDown);
                                }
                                else if(VKCode == VK_LEFT)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Left, IsDown);
                                }
                                else if(VKCode == VK_UP)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Up, IsDown);
                                }
                                else if(VKCode == VK_DOWN)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Down, IsDown);
                                }
                                else if(VKCode == VK_SPACE)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Space, IsDown);
                                }
                                else if(VKCode == VK_ESCAPE)
                                {
                                    ProcessKeyboardInput(&KeyboardInput.Esc, IsDown);
                                }
                            }
                        } break;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                
                game_offscreen_buffer Buffer = {};
                
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch;
                Buffer.BytesPerPixel = 4;
                
                Game.UpdateAndRender(&GameMemory, &Buffer, &GameInput);
                //HandleDebugCounters(&GameMemory);
                
                HDC DeviceContext = GetDC(Window);
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext,Dimension.Width, Dimension.Height, &GlobalBackbuffer);
                
                // TIMER // 
                
                s64 EndCycleCount = __rdtsc();
                
                LARGE_INTEGER EndCounter = Win32GetWallClock();
                
                s64 CyclesElapsed = EndCycleCount - LastCycleCount;
                s64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                f32 MSPerFrame = (f32)((1000.0f*(f32)CounterElapsed) / (f32)PerfCountFrequency);
                f32 FPS = (f32)PerfCountFrequency / (f32)CounterElapsed;
                f32 MCPF = (f32)(CyclesElapsed / (1000.0f * 1000.0f));
                
#if 0
                char Buffer0[256];
                sprintf(Buffer0, "\nMilliseconds/frame: %fms %fFPS\n %fmc/f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(Buffer0);
#endif
                
                if(LockedFPS)
                { 
                    if(MSPerFrame < TargetMS)
                    {
                        SleepTime = (u32)(TargetMS - MSPerFrame); 
                    }
                }
                else
                {
                    GameInput.dtForFrame = MSPerFrame*InverseMS;
                }
                
                
                
                ReleaseDC(Window, DeviceContext);
                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
            
        }
        else
        {
            
        }
    }
    else
    {
        
    }
    
    return 0;
}