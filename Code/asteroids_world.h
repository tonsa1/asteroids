#if !defined(ASTEROIDS_WORLD_H)

void
InitializeWorld(world *World, f32 ScreenXInMeters, f32 ScreenYInMeters)
{
    World->ChunkDim = V2(ScreenXInMeters, ScreenYInMeters);
    World->YCount = 8;
    World->XCount = 8;
}

internal world_chunk *
GetWorldChunk(world *World, v2_i Chunk, memory_arena *Arena = 0)
{
    u32 HashValue = 19*Chunk.x + 7*Chunk.y;
    u32 HashSlot = HashValue & (ArrayCount(World->ChunkHashes) - 1);
    Assert(HashSlot < ArrayCount(World->ChunkHashes));
    
    world_chunk *WorldChunk = World->ChunkHashes + HashSlot;
    do
    {
        if(V2_iEquals(WorldChunk->Chunk, Chunk))
        {
            break;
        }
        
        if(Arena && (!WorldChunk->NextInHash))
        {
            world_chunk *NewChunk = PushStruct(Arena, world_chunk);
            *NewChunk = {};
            NewChunk->Chunk = Chunk;
            NewChunk->EntityCount = 0;
            
            WorldChunk->NextInHash = NewChunk;
            WorldChunk = NewChunk;
            
            break;
        }
        
        WorldChunk = WorldChunk->NextInHash;
    } while (WorldChunk);
    
    return WorldChunk;
}

#define ASTEROIDS_WORLD_H
#endif