#if !defined(ASTEROIDS_H)

/*


Asteroids
HP
Shooting


----#DONE----

Rasterization
Rotation
Physics movement
Collision -- Okayish?
IO
BMP sprites
Vectors
*/

#define BITMAP_BYTES_PER_PIXEL 4
#define Pi32 3.14159265359f

#define Maximum(A, B) ((A) > (B) ? A : B)
#define Minimum(A, B) ((A) > (B) ? B : A)

#include "asteroids_platform.h"
#include "asteroids_math.h"
#include "asteroids_intrinsics.h"

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
    
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

internal temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    
    Arena->TempCount++;
    
    return Result;
}

internal void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    
    Arena->Used = TempMem.Used;
    Arena->TempCount--;
}

internal void
CheckMemory(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

void
InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type));
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))

internal void * 
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return Result;
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size)
{
    Result->Size = Size;
    Result->Base = (u8 *)PushSize_(Arena, Size);
    Result->Used = 0;
    Result->TempCount = 0;
}

struct loaded_bitmap
{
    s32 Width;
    s32 Height;
    s32 Pitch;
    u8 *Pixels;
};

struct line
{
    v4 Color;
    v2 Start;
    v2 End;
    v2 Normal;
    v2 Vector;
};

struct bezier
{
    union
    {
        struct
        {
            v2 A;
            v2 B;
            v2 C;
            v2 D;
        };
        
        v2 points[4];
    };
};

struct line_mesh
{
    u32 LineCount;
    line *Lines;
};

#include "asteroids_assets.h"

enum hitbox_type
{
    HitboxType_None,
    HitboxType_LineMesh,
    HitboxType_Rectangle,
    HitboxType_Radius,
};

enum collision_flags
{
    Flag_None = 0x0,
    Flag_Player = 0x1,
    Flag_Object = 0x2,
    Flag_Asteroid = 0x4,
    Flag_Bullet = 0x8,
};

enum entity_type
{
    EntityType_None,
    EntityType_Player,
    EntityType_Object,
    EntityType_Asteroid,
    EntityType_Bullet,
    EntityType_Sprite,
    
    EntityType_Camera,
};

enum asteroid_type
{
    AsteroidType_None,
    AsteroidType_Small,
    AsteroidType_Big,
};

struct coordinate_basis
{
    v2 Origin;
    v2 X;
    v2 Y;
    f32 Angle;
    f32 Scale;
};

struct entity
{
    entity_type EntityType;
    asteroid_type AsteroidType;
    hitbox_type HitboxType;
    collision_flags CollisionFlags;
    game_asset_id GameAssetID;
    
    u32 EntityIndex;
    u32 CollisionEntityType;
    u32 EntityID;
    
    f32 MaxTravelDistance;
    f32 TravelDistance;
    f32 PlayerSpeed;
    v2 P;
    v2_i Chunk;
    v2 dP;
    v2 ddP;
    f32 ddR;
    v2 Dim;
    
    v4 Color;
    coordinate_basis Basis;
    
    b32 Movable;
    b32 Collides;
};


struct transient_state
{
    b32 IsInitialized;
    memory_arena TranArena;
    
    game_assets *Assets;
    
    platform_work_queue *Queue;
};

struct world_chunk
{
    v2_i Chunk;
    u32 EntityCount;
    entity Entities[128];
    u32 AsteroidPoints;
    world_chunk *NextInHash;
};

struct world
{
    v2 WorldCenter;
    v2 ChunkDim;
    u32 XCount;
    u32 YCount;
    
    f32 BulletCooldown;
    entity *BulletEntity;
    
    world_chunk ChunkHashes[8];
};

struct game_buffer
{
    u8 *Memory;
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
    u32 Pitch;
    f32 MetersToPixels;
    f32 PixelsToMeters;
};

struct game_state
{
    memory_arena Arena;
    
    game_buffer GameBuffer;
    world *GameWorld;
    
    entity CameraEntity;
    v2 CameraOffsetForFrame;
    
    f32 MetersToPixels;
    f32 Angle;
    
    u32 AsteroidSpawnCount;
    v2 *AsteroidSpawns;
    u32 NextID;
    
    v2 *Points;
};

world ResetWorld;

global_variable platform_add_work_queue_entry *PlatformAddWorkQueueEntry;
global_variable platform_do_next_work_queue_entry *PlatformDoNextWorkQueueEntry;
global_variable platform_complete_all_work_queue_work *PlatformCompleteAllWorkQueueWork;
global_variable debug_platform_read_entire_file *DEBUGReadEntireFile;
global_variable platform_print_debug_string *PlatformPrintString;

#include "asteroids_world.h"
#include "asteroids_assets.cpp"
#include "asteroids_collision.h"
#include "asteroids_entity.h"
#include "asteroids_render.h"
#include "vector_graphics.h"
#define ASTEROIDS_H
#endif