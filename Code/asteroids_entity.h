#if !defined(ASTEROIDS_ENTITY_H)

internal void
InitializeCamera(game_state *GameState, v2 Pos, v2 Dim, entity_type EntityType, v2_i Chunk)
{
    entity *PlayerEntity = &GameState->CameraEntity;
    
    PlayerEntity->EntityType = EntityType;
    PlayerEntity->P = Pos;
    PlayerEntity->dP = {};
    PlayerEntity->Dim = Dim;
    PlayerEntity->EntityIndex = 0;
    PlayerEntity->Chunk = Chunk;
}

coordinate_basis
CreateCoordinateBasis()
{
    coordinate_basis Result = {};
    
    return Result;
}

internal u32
AddEntity(world *World, game_state *GameState, v2 Pos, v2 Dim,
          entity_type Type, collision_flags Flags, game_asset_id GameAssetID,
          hitbox_type HitboxType, v2_i Chunk,
          b32 Movable, b32 Collides)
{
    world_chunk *WorldChunk = GetWorldChunk(World, Chunk);
    u32 EntityIndex = {};
    if(WorldChunk)
    {
        EntityIndex = WorldChunk->EntityCount;
        if(EntityIndex <= ArrayCount(WorldChunk->Entities))
        {
            entity *Entity = WorldChunk->Entities + EntityIndex;
            
            Entity->EntityType = Type;
            Entity->HitboxType = HitboxType;
            Entity->CollisionFlags = Flags;
            Entity->P = Pos;
            Entity->Chunk = Chunk;
            Entity->dP = {};
            Entity->Dim = Dim;
            Entity->EntityIndex = EntityIndex;
            Entity->Movable = Movable;
            Entity->Collides = Collides;
            Entity->Basis.X = V2i(1,0);
            Entity->Basis.Y = V2i(0,1);
            Entity->EntityID = GameState->NextID++;
            Entity->GameAssetID = GameAssetID;
            ++WorldChunk->EntityCount;
        }
    }
    
    return EntityIndex;
}

internal void
RemoveEntity(world_chunk *Chunk, entity *Entity)
{
    u32 EntityIndex = Entity->EntityIndex;
    u32 LastOnListIndex = Chunk->EntityCount - 1;
    
    if(EntityIndex != LastOnListIndex)
    {
        entity LastOnListEntity = Chunk->Entities[LastOnListIndex];
        LastOnListEntity.EntityIndex = EntityIndex;
        
        Chunk->Entities[EntityIndex] = LastOnListEntity;
        Chunk->Entities[LastOnListIndex] = {};
        
    }
    else
    {
        Chunk->Entities[EntityIndex] = {};
    }
    
    --Chunk->EntityCount;
}

internal void
RemoveEntity(world_chunk *Chunk, u32 EntityID)
{
    for(u32 TestEntity = 0;
        TestEntity < Chunk->EntityCount;
        ++TestEntity)
    {
        entity *Entity = Chunk->Entities + TestEntity;
        
        if(Entity)
        {
            if(Entity->EntityID == EntityID)
            {
                RemoveEntity(Chunk, Entity);
                return;
            }
        }
    }
}

internal entity *
GetEntity(world_chunk *Chunk, u32 EntityID)
{
    for(u32 TestEntity = 0;
        TestEntity < Chunk->EntityCount;
        ++TestEntity)
    {
        entity *Entity = Chunk->Entities + TestEntity;
        
        if(Entity)
        {
            if(Entity->EntityID == EntityID)
            {
                return Entity;
            }
        }
    }
    
    return 0;
}

internal entity_type
GetEntityType(world_chunk *Chunk, u32 EntityID)
{
    for(u32 TestEntity = 0;
        TestEntity < Chunk->EntityCount;
        ++TestEntity)
    {
        entity *Entity = Chunk->Entities + TestEntity;
        
        if(Entity)
        {
            if(Entity->EntityID == EntityID)
            {
                return Entity->EntityType;
            }
        }
    }
    
    return EntityType_None;
}


internal void
CreateAsteroid(game_state *GameState, game_assets *Assets, world *World, world_chunk *Chunk,
               asteroid_type AsteroidType, f32 Scale, v2 OldVelocity = {}, v2 OldPosition = {})
{
    if (Chunk->EntityCount < ArrayCount(Chunk->Entities))
    {
        
        u32 RandomAsteroidID = (RandomValue() % MAX_ASTEROID_MESH_COUNT);
        Assert(RandomAsteroidID >= GAI_LineMeshAsteroidStart && RandomAsteroidID <= GAI_LineMeshAsteroidEnd);
        game_asset_id AssetID = (game_asset_id)(GAI_LineMeshAsteroidStart + RandomAsteroidID);
        
        entity *Entity = Chunk->Entities + AddEntity(World, GameState,
                                                     V2(0,0),
                                                     V2(1,1),
                                                     EntityType_Asteroid,
                                                     (collision_flags)(Flag_Bullet | Flag_Player),
                                                     AssetID,
                                                     HitboxType_LineMesh,
                                                     Chunk->Chunk,
                                                     true, true);
        
        Entity->Color = V4(1,1,1,1);
        Entity->AsteroidType = AsteroidType_Big;
        
        line_mesh *LineMesh = GetLineMesh(Assets, AssetID);
        
        v2 RandomOffset = {};
        u32 RandomNumber = 0;
        f32 RandomSpeed = 0;
        f32 RandomSpeedR = 0;
        f32 RandScale = 0;
        v2 ddP = {};
        v2 P = {};
        
        if(AsteroidType == AsteroidType_Big)
        {
            RandomNumber = RandomValue() % GameState->AsteroidSpawnCount;
            RandomOffset = V2(RandomValue() % 10 + -5.0f, RandomValue() % 10 + -5.0f);
            RandomSpeed = (f32)(RandomValue() % 10 + 2);
            RandomSpeedR = RandomSpeed * 0.15f;
            RandomSpeed *= 0.175f;
            RandomSpeed = (RandomSpeed < .1f) ? .1f : RandomSpeed;
            RandScale = (f32)(RandomValue() % 10);
            RandScale *= 0.1f;
            RandScale += 0.75f;
            RandScale = (RandScale < 3.0f) ? 3.0f : RandScale;
            RandScale = (RandScale > 7.0f) ? 7.0f : RandScale;
            
            P = GameState->AsteroidSpawns[RandomNumber];
            ddP = (0.02f + RandomSpeed)*Normalize((World->WorldCenter + RandomOffset) - P);
        }
        else if(AsteroidType == AsteroidType_Small)
        {
            srand(GameState->NextID);
            RandomOffset = V2(RandomValue() % 2 + -1.0f, RandomValue() % 1 + -1.0f);
            RandomSpeed = (f32)(RandomValue() % 10 + 2);;
            RandomSpeedR = RandomSpeed * 0.15f;
            RandomSpeed *= 0.175f;
            RandomSpeed = (RandomSpeed < .1f) ? .1f : RandomSpeed;
            RandScale = (f32)(RandomValue() % 5);
            RandScale *= 0.1f;
            RandScale += 0.3f;
            RandScale = (RandScale < 1.0f) ? 1.0f : RandScale;
            RandScale = (RandScale > 3.0f) ? 3.0f : RandScale;
            
            ddP = OldVelocity;
            P = OldPosition + RandomOffset*0.4f;
        }
        
        Entity->AsteroidType = AsteroidType;
        Entity->Basis.Scale = Scale * RandScale;
        Entity->P = P;
        Entity->MaxTravelDistance = 25.0f;
        Entity->ddP = ddP;
        Entity->ddR = RandomSpeedR;
        Entity->TravelDistance = 0;
        Entity->Collides = true;
    }
}


internal void
CreateBullet(game_state *GameState, game_assets *Assets, world *World, world_chunk *Chunk, entity *PlayerEntity)
{
    if (Chunk->EntityCount < ArrayCount(Chunk->Entities) && World->BulletCooldown == 0.0f)
    {
        World->BulletCooldown -= 0.5f;
        entity *Entity = Chunk->Entities + AddEntity(World, GameState,
                                                     V2(0,0),
                                                     V2(1,1),
                                                     EntityType_Bullet,
                                                     (collision_flags)(Flag_Asteroid),
                                                     GAI_LineMeshBullet,
                                                     HitboxType_LineMesh,
                                                     Chunk->Chunk,
                                                     true, true);
        
        line_mesh *LineMesh = GetLineMesh(Assets, GAI_LineMeshBullet);
        line Line = LineMesh->Lines[0];
        v2 RotatedEnd = Line.End.x*PlayerEntity->Basis.X + Line.End.y*PlayerEntity->Basis.Y;
        
        Entity->Basis.Scale = 0.15f;
        Entity->P = PlayerEntity->P + RotatedEnd;
        Entity->MaxTravelDistance = 15.0f;
        Entity->ddP = 10.3f*Normalize(PlayerEntity->Basis.Y);
        Entity->ddR = 0.1f;
        Entity->TravelDistance = 0;
        Entity->Collides = true;
        Entity->Color = V4(1,0,0,1);
    }
}

internal void
BreakAsteroid(game_state *GameState, game_assets *Assets, world *World, world_chunk *Chunk, u32 EntityID)
{
    
#if 1    
    entity *Entity = GetEntity(Chunk, EntityID);
    
    if(Entity)
    {
        switch(Entity->AsteroidType)
        {
            case AsteroidType_Small:
            {
                RemoveEntity(Chunk, Entity);
                CreateAsteroid(GameState, Assets, World, Chunk, AsteroidType_Big, 1.0f);
            } break;
            
            case AsteroidType_Big:
            {
                v2 OldVelocity = Entity->ddP;
                v2 OldPos = Entity->P;
                f32 SpawnsLeft = Entity->Basis.Scale;
                RemoveEntity(Chunk, Entity->EntityID);
                
#if 1
                for(u32 AsteroidIndex = 0;
                    SpawnsLeft > 0.0f;
                    ++AsteroidIndex)
                {
                    v2 RandomOffset = V2((f32)(RandomValue() % 10) * 0.1f , (f32)(RandomValue() % 10) * 0.1f);
                    CreateAsteroid(GameState, Assets, World, Chunk, AsteroidType_Small, SpawnsLeft, OldVelocity, OldPos + RandomOffset);
                    SpawnsLeft -= 1.0f;
                }
#endif
            } break;
        }
    }
#endif
}

void
MovePlayer(memory_arena *Arena, game_assets *Assets, world *World, game_state *GameState, world_chunk *CameraChunk, entity *Entity, u32 EntityIndex, f32 dt)
{
    v2 OldP = Entity->P;
    Entity->ddP += -Entity->dP*5.55f; // POOR "FRICTION"
    
    v2 EntitydP = (0.5f*Entity->ddP*Square(dt) +
                   Entity->dP*dt);
    
    v2 EntityNewP = EntitydP + Entity->P;
    Entity->dP = Entity->ddP*dt + Entity->dP;
    
    u32 CollisionEntityID = LineMeshCollisionCheck(GameState,  Entity, CameraChunk, Assets);
    
    u32 CurrentEntityID = Entity->EntityID;
    if(CollisionEntityID)
    {
        if(GetEntityType(CameraChunk, CollisionEntityID) == EntityType_Asteroid)
        {
            RemoveEntity(CameraChunk, Entity->EntityID);
            world_chunk *Chunk = GetWorldChunk(World, Entity->Chunk);
            //BreakAsteroid(GameState, World, Chunk, CollisionEntityID);
            Entity = GetEntity(Chunk, CurrentEntityID);
            return;
        }
    }
    
    Entity->P += EntitydP;
    
    v2 Pos = Entity->P;
    
    Entity->Basis.Angle += dt*Entity->ddR;
    Entity->Basis.X = V2(Cos(Entity->Basis.Angle), Sin(Entity->Basis.Angle));
    Entity->Basis.Y = Perp(Entity->Basis.X);
    
    Entity->ddP = Entity->Basis.Y;
    Entity->ddP *= Entity->PlayerSpeed;
    
    Entity->ddR = 0.0f;
    Entity->PlayerSpeed = 0.0f;
    //Entity->Multiplier = 0.0f;
    
    // NOTE(Tony): Disable changing screens for now.
#if 0    
    rect2 ChunkRect = HalfDimRect2(World->ChunkDim / 2, World->ChunkDim);
    if (!IsPointInsideRect(Pos, ChunkRect))
    {
        v2_i Chunk = GameState->CameraEntity.Chunk;
        world_chunk *OldChunk = GetWorldChunk(World, Chunk);
        
        if(Pos.y < 0.0f)
        {
            Chunk.y -= 1;
            Pos.y += World->ChunkDim.y;
        }
        else if(Pos.y > World->ChunkDim.y)
        {
            Chunk.y += 1;
            Pos.y -= World->ChunkDim.y;
        }
        
        if(Pos.x < 0)
        {
            Chunk.x -= 1;
            Pos.x += World->ChunkDim.x;
        }
        else if(Pos.x > World->ChunkDim.x)
        {
            Chunk.x += 1;
            Pos.x -= World->ChunkDim.x;
        }
        
        if(Entity->EntityType == EntityType_Player)
        {
            GameState->CameraEntity.Chunk = Chunk;
            
        }
        
        world_chunk *NewChunk = GetWorldChunk(World, Chunk, Arena);
        u32 OldEntityIndex = Entity->EntityIndex;
        u32 LastOnListIndex = OldChunk->EntityCount - 1;
        entity CurrentEntity = *Entity;
        entity LastOnListEntity = OldChunk->Entities[LastOnListIndex];
        
        OldChunk->Entities[LastOnListIndex] = {};
        
        CurrentEntity.Chunk = Chunk;
        CurrentEntity.P = Pos;
        CurrentEntity.EntityIndex = NewChunk->EntityCount;
        LastOnListEntity.EntityIndex = OldEntityIndex;
        
        NewChunk->Entities[NewChunk->EntityCount] = CurrentEntity;
        OldChunk->Entities[OldEntityIndex] = LastOnListEntity;
        
        ++NewChunk->EntityCount;
        --OldChunk->EntityCount;
        
    }
#endif
    
}

internal void
MoveAsteroid(game_state *GameState, game_assets *Assets, world *World, world_chunk *Chunk, u32 EntityID, f32 dt)
{
    entity *Entity = GetEntity(Chunk, EntityID);
    
    if(Entity)
    {
        Entity->P += dt*Entity->ddP;
        Entity->TravelDistance += dt*Length(Entity->ddP);
        Entity->Basis.Angle += dt*Entity->ddR;
        
        Entity->Basis.X = V2(Cos(Entity->Basis.Angle), Sin(Entity->Basis.Angle))*Entity->Basis.Scale;
        Entity->Basis.Y = Perp(Entity->Basis.X);
        
        u32 CollisionEntityID = LineMeshCollisionCheck(GameState, Entity, Chunk, Assets);
        entity *CollisionEntity = GetEntity(Chunk, CollisionEntityID);
        
        
        if(CollisionEntity)
        {
            if(CollisionEntity->EntityType == EntityType_Player)
            {
                RemoveEntity(Chunk, CollisionEntityID);
                //BreakAsteroid(GameState, World, Chunk, CollisionEntityID);
                Entity = GetEntity(Chunk, EntityID);
            }
        }
        
        if(Entity->TravelDistance >= Entity->MaxTravelDistance)
        {
            RemoveEntity(Chunk, EntityID);
            CreateAsteroid(GameState, Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
        }
    }
}

internal void
MoveBullet(game_state *GameState, game_assets *Assets,  world *World, world_chunk *Chunk, u32 EntityID, f32 dt)
{
    entity *Entity = GetEntity(Chunk, EntityID);
    
    if(Entity)
    {
        Entity->P += dt*Entity->ddP;
        Entity->TravelDistance += dt*Length(Entity->ddP);
        Entity->Basis.Angle += dt*Entity->ddR;
        
        Entity->Basis.X = V2(Cos(Entity->Basis.Angle), Sin(Entity->Basis.Angle))*Entity->Basis.Scale;
        Entity->Basis.Y = Perp(Entity->Basis.X);
        
        u32 CollisionEntityID = LineMeshCollisionCheck(GameState, Entity, Chunk, Assets);
        entity *CollisionEntity = GetEntity(Chunk, CollisionEntityID);
        
        if(CollisionEntity)
        {
            if(CollisionEntity->EntityType == EntityType_Asteroid)
            {
                //CollisionEntity->AsteroidType = AsteroidType_Small;
                BreakAsteroid(GameState, Assets, World, Chunk, CollisionEntityID);
                RemoveEntity(Chunk, EntityID);
            }
        }
        
        if(Entity)
        {
            if(Entity->TravelDistance >= Entity->MaxTravelDistance)
            {
                RemoveEntity(Chunk, Entity->EntityID);
            }
        }
    }
}

#define ASTEROIDS_ENTITY_H
#endif