#if !defined(ASTEROIDS_WORLD_H)



internal line
CreateLine(v2 Start, v2 End)
{
    line Line = {};
    
    Line.Start = Start;
    Line.End = End;
    Line.Vector = Line.End - Line.Start;
    Line.Normal = Normalize(Perp(Line.Vector));
    
    return Line;
}

internal
u32 RandomValue()
{
    // TODO(Tony): Replace this with less horrible rng
    u32 Result = rand();
    
    return Result;
}

internal
v2 RandomOffset()
{
    v2 Result;
    
    Result.x = (f32)(RandomValue() % 10) * 0.1f;
    Result.y = (f32)(RandomValue() % 10) * 0.1f;
    
    return Result;
}

internal
v2 RandomOffsetBilateral(f32 Scale = 1.0f)
{
    v2 Result;
    
    Result.x = ((f32)(RandomValue() % 20) * 0.1f - 1.0f) * Scale;
    Result.y = ((f32)(RandomValue() % 20) * 0.1f - 1.0f) * Scale;
    
    return Result;
}

internal 
v2 PointToWorld(entity *Entity, v2 P)
{
    v2 Result;
    
    Result = (Entity->P + P.x*Entity->Basis.X + P.y*Entity->Basis.Y);
    
    return Result;
}

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