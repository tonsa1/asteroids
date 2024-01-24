#if !defined(ASTEROIDS_PLATFORM_H) 

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <intrin.h>
#include <cstdlib>
#include <ctime>
#include <float.h>

#define internal static 
#define local_persist static 
#define global_variable static

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;

typedef size_t memory_index;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define S32Min ((u32)0x80000000)
#define S32Max ((u32)0x7fffffff)
#define S64Max ((u64)-1)

#define f32Max FLT_MAX

#define Assert(Condition) do { if (!(Condition))  __debugbreak(); } while (0)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct cycle_count
{
    u64 Cycles;
    u64 Hits;
};

enum DebugCycleCounter
{
    DebugCycleCounter_DrawLine,
    DebugCycleCounter_PerPixel,
};

#define START_COUNTER(name) u64  name = __rdtsc();
#define END_COUNTER(name) Memory->Counters[DebugCycleCounter_##name].Cycles += __rdtsc() - name; Memory->Counters[DebugCycleCounter_##name].Hits += 1;



inline u32
SafeTruncateInt64(u32 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    u32 Value32  = (u32)Value;
    return Value32;
}

struct debug_read_file_result
{
    u32 ContentsSize;
    void *Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

struct game_button_state
{
    s32 HalfTransitionCount;
    b32 EndedDown;
    
};

struct game_controller_input
{
    union
    {
        game_button_state Buttons[8];
        struct 
        {
            game_button_state Left;
            game_button_state Right;
            game_button_state Up;
            game_button_state Down;
            game_button_state Space;
            game_button_state Esc;
        };
    };
};



struct game_offscreen_buffer
{
    void *Memory;
    u32 Width;
    u32 Height;
    u32 Pitch; 
    u32 BytesPerPixel;
};

struct game_input
{
    game_controller_input *KeyboardInput;
    
    f32 dtForFrame;
};

struct platform_work_queue_entry;
struct platform_work_queue;

#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(void *Data, int LogicalThreadIndex)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_work_queue_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
typedef b32 platform_do_next_work_queue_entry(platform_work_queue *Queue, u32 LogicalThreadIndex); 
typedef void platform_complete_all_work_queue_work(platform_work_queue *Queue, u32 LogicalThreadIndex);


typedef void platform_print_debug_string(char *String, u32 Thread);


struct game_memory
{
    b32 IsInitialized;
    
    platform_add_work_queue_entry *PlatformAddWorkQueueEntry;
    platform_do_next_work_queue_entry *PlatformDoNextWorkQueueEntry;
    platform_complete_all_work_queue_work *PlatformCompleteAllWorkQueueWork;
    platform_work_queue_callback *DoStringWork;
    platform_work_queue *Queue;
    platform_print_debug_string *PrintString;
    
    u32 PermanentStorageSize;
    void *PermanentStorage;
    
    u32 TransientStorageSize;
    void *TransientStorage;
    
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    
    cycle_count Counters[256];
    s32 ScreenDimensionX;
    s32 ScreenDimensionY;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_offscreen_buffer *Buffer,game_input *Input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define ASTEROIDS_PLATFORM_H
#endif
