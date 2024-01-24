#include "asteroids.h"

struct m4x4
{
    f32 m[4][4];
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    m4x4 test = {
        1,2,3,4,
        1,2,3,4,
        1,2,3,4,
        1,2,3,4,
    };
    
    PlatformAddWorkQueueEntry = Memory->PlatformAddWorkQueueEntry;
    PlatformDoNextWorkQueueEntry = Memory->PlatformDoNextWorkQueueEntry;
    PlatformCompleteAllWorkQueueWork = Memory->PlatformCompleteAllWorkQueueWork;
    PlatformPrintString = Memory->PrintString;
    u32 GameStateSize = sizeof(game_state);
    Assert(GameStateSize <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    game_buffer *GameBuffer = &GameState->GameBuffer;
    world *World = GameState->GameWorld;
    
    f32 ScreenXInMeters = 20.0f;
    v2 ScreenDimensions = V2i(Memory->ScreenDimensionX, Memory->ScreenDimensionY);
    f32 ScreenYInMeters = ScreenXInMeters * (ScreenDimensions.y / ScreenDimensions.x);
    f32 MetersToPixels = ScreenDimensions.x / ScreenXInMeters;
    GameBuffer->MetersToPixels = MetersToPixels;
    GameBuffer->PixelsToMeters = 1.0f/MetersToPixels;
    v2 ScreenCenter  = V2(ScreenDimensions.x/2, ScreenDimensions.y/2);
    v2 ScreenCenterInMeters = ScreenCenter/MetersToPixels;
    GameState->CameraOffsetForFrame = V2(0,0);
    
    
    entity *PlayerEntity = 0;
    //~ INIT
    if(!Memory->IsInitialized)
    {
        InitializeArena(&GameState->Arena, (Memory->PermanentStorageSize - sizeof(game_state)),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        
        
        GameState->GameWorld = PushStruct(&GameState->Arena, world);
        World = GameState->GameWorld;
        InitializeWorld(World, ScreenXInMeters, ScreenYInMeters);
        
        
        
        v2_i StartChunk = V2_i(0,0);
        world_chunk *Chunk = GetWorldChunk(World, StartChunk, &GameState->Arena);
        World->WorldCenter = ScreenCenterInMeters;
        GameState->NextID = 1;
        DEBUGReadEntireFile = Memory->DEBUGPlatformReadEntireFile;
        
        InitializeCamera(GameState,
                         V2(0,0),
                         V2(0,0), EntityType_Camera,
                         StartChunk);
        
        Memory->IsInitialized = true;
    }
    
    
    Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(transient_state),
                        (u8 *)Memory->TransientStorage + sizeof(transient_state));
        
        
        TranState->Assets = AllocateAssets(TranState, Kilobytes(256));
        
        v2_i StartChunk = V2_i(0,0);
        world_chunk *Chunk = GetWorldChunk(World, StartChunk, &GameState->Arena);
        
        PlayerEntity = Chunk->Entities + 
            AddEntity(World, GameState, 
                      V2(2.8f, 2.65f),
                      V2(0.0f,0.0f),
                      EntityType_Player,
                      (collision_flags)(Flag_Asteroid),
                      GAI_LineMeshPlayer,
                      HitboxType_LineMesh,
                      StartChunk,
                      true, true);
        
        TranState->Queue = Memory->Queue;
        
        PlayerEntity->Color = V4(1,1,1,1);
        
#if 0
        entity *ImageEntity = Chunk->Entities + AddEntity(World, GameState, 
                                                          V2(5.0f, 5.0f),
                                                          V2(1.0f,1.0f),
                                                          EntityType_Sprite,
                                                          (collision_flags)(Flag_None),
                                                          GAI_CharacterA,
                                                          HitboxType_None,
                                                          StartChunk,
                                                          false, false);
#endif
        //
        
        GameState->AsteroidSpawnCount = 16;
        v2 *Spawns = PushArray(&GameState->Arena, GameState->AsteroidSpawnCount, v2);
        GameState->AsteroidSpawns = Spawns;
        
        f32 OverY = GameState->GameWorld->ChunkDim.y + 2.0f;
        f32 OverX = GameState->GameWorld->ChunkDim.x + 2.0f;
        
        Spawns[0] = V2(-2.0f,1.0f);
        Spawns[1] = V2(-2.0f,3.0f);
        Spawns[2] = V2(-2.0f,9.0f);
        Spawns[3] = V2(2.0f,-2.0f);
        Spawns[4] = V2(17.0f,-2.0f);
        Spawns[5] = V2(8.0f,-2.0f);
        Spawns[6] = V2(4.0f,-2.0f);
        Spawns[7] = V2(OverX,2.0f);
        Spawns[8] = V2(OverX,7.0f);
        Spawns[9] = V2(OverX,3.0f);
        Spawns[10] = V2(OverX,9.0f);
        Spawns[11] = V2(OverX,6.0f);
        Spawns[12] = V2(2.0f,OverY);
        Spawns[13] = V2(10.0f,OverY);
        Spawns[14] = V2(18.0f,OverY);
        Spawns[15] = V2(13.0f,OverY);
        Spawns[16] = V2(7.0f,OverY);
        
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
#if 0
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (RandomValue() % 1 + 0.5f));
#endif
        
        GameState->CameraOffsetForFrame = -ScreenCenterInMeters;
        GameState->GameBuffer.Width = Buffer->Width;
        GameState->GameBuffer.Height = Buffer->Height;
        GameState->GameBuffer.BytesPerPixel = 4;
        GameState->GameBuffer.Pitch = Buffer->Width * 4;
        GameState->GameBuffer.Memory = (u8 *)PushArray(&GameState->Arena,
                                                       Buffer->Width * Buffer->Height, u32);
        
        ResetWorld = *GameState->GameWorld;
        
        TranState->IsInitialized = true;
    }
    
    local_persist b32 Esc = false;
    if(Input->KeyboardInput->Esc.EndedDown)
    {
        Esc = true;
    }
    
    if(Esc == 1 && !Input->KeyboardInput->Esc.EndedDown)
    {
        *GameState->GameWorld = ResetWorld;
        Esc = false;
    }
    
    
    
    temporary_memory TempMem = BeginTemporaryMemory(&TranState->TranArena);
    Assert(TempMem.Arena);
    
    ResetBufferBlack(GameBuffer);
    
    f32 dt = Input->dtForFrame;
    World->BulletCooldown += dt;
    
    if(World->BulletCooldown > 0.0f)
    {
        World->BulletCooldown = 0.0f;
    }
    world_chunk *CameraChunk = GetWorldChunk(World, GameState->CameraEntity.Chunk);
    
    //~ UPDATE VELOCITY
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        if(TestEntity->Movable)
        {
            switch(TestEntity->EntityType)
            {
                case EntityType_Player:
                {
                    PlayerEntity = TestEntity;
                    v2 PlayerddP = {};
                    f32 PlayerSpeed = 5.0f; // m/s^2
                    f32 Multiplier = 38.0f;
                    f32 RotationSpeed = 5.6f;
                    
                    
#if 0                    
                    PlayerSpeed = 1.0f;
                    RotationSpeed = 0.6f;
                    Multiplier = 2.f;
#endif
                    
                    if(Input->KeyboardInput->Space.EndedDown)
                    {
                        CreateBullet(GameState,TranState->Assets, World, CameraChunk, TestEntity);
                    }
                    
                    if(Input->KeyboardInput->Left.EndedDown)
                    {
                        TestEntity->ddR = -RotationSpeed;
                    }
                    
                    if(Input->KeyboardInput->Right.EndedDown)
                    {
                        TestEntity->ddR = RotationSpeed;
                    }
                    
                    if(Input->KeyboardInput->Up.EndedDown)
                    {
                        TestEntity->PlayerSpeed = Multiplier;
                    }
                    
                    if(Input->KeyboardInput->Down.EndedDown)
                    {
                        TestEntity->PlayerSpeed = -Multiplier;
                    }
                    
                    if(PlayerddP.x != 0.0f && PlayerddP.y != 0.0f)
                    {
                        f32 playerddPLength = SquareRoot(Inner(PlayerddP, PlayerddP));
                        TestEntity->ddP *= 2.0f / playerddPLength;
                    }
                    
                    f32 Boost = 14.0f;
                    
                    f32 ForceFieldWidth = .1f;
                    if(TestEntity->P.x < ForceFieldWidth)
                    {
                        TestEntity->ddP += Boost*V2(ForceFieldWidth - Absolute(TestEntity->P.x), 0.0f);
                    }
                    if(TestEntity->P.x > GameState->GameWorld->ChunkDim.x - ForceFieldWidth)
                    {
                        
                        TestEntity->ddP -=
                            Boost*V2(TestEntity->P.x - (GameState->GameWorld->ChunkDim.x - ForceFieldWidth), 0.0f);
                    }
                    
                    if(TestEntity->P.y < ForceFieldWidth)
                    {
                        TestEntity->ddP += Boost*V2(0.0f, ForceFieldWidth - Absolute(TestEntity->P.y));
                    }
                    
                    if(TestEntity->P.y > GameState->GameWorld->ChunkDim.y - ForceFieldWidth)
                    {
                        TestEntity->ddP -= 
                            Boost*V2(0.0f, TestEntity->P.y - (GameState->GameWorld->ChunkDim.y - ForceFieldWidth));
                    }
                } break;
            }
        }
    }
    
    u32 CurrentEntityCount = 0;
    //~ MOVE ENTITIES
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        CurrentEntityCount = CameraChunk->EntityCount;
        if(TestEntity->Movable)
        {
            switch(TestEntity->EntityType)
            {
                case EntityType_Player:
                {
                    MovePlayer(&GameState->Arena,TranState->Assets, World, GameState, CameraChunk, TestEntity, TestEntityIndex, dt);
                    CameraChunk = GetWorldChunk(World, GameState->CameraEntity.Chunk);
                }break;
                
                case EntityType_Asteroid:
                {
                    MoveAsteroid(GameState,TranState->Assets, World, CameraChunk, TestEntity->EntityID, dt);
                }break;
                
                case EntityType_Bullet:
                {
                    MoveBullet(GameState,TranState->Assets, World, CameraChunk, TestEntity->EntityID, dt);
                }break;
            }
        }
        
        if(CurrentEntityCount < CameraChunk->EntityCount)
        {
            --TestEntityIndex;
        }
    }
    
    
    if (PlayerEntity)
    {
        //entity *Entity = Asteroid;
        
        entity TempEntity = *PlayerEntity;
        
#if 0
        game_asset_id ID = Entity->GameAssetID;
        ID = GAI_LineMeshBox;
        line_mesh *Mesh = GetLineMesh(TranState->Assets, ID);
#endif
        
        //TempEntity.Basis.Angle = 5.9f;
        
        //CreateMmozeiko(TempMem, &TempEntity, GameBuffer);
    }
    
    //~ RENDER
    
    f32 LineWidth = 1.0f;
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        line_mesh *TestEntityLineMesh = 0;
        switch(TestEntity->EntityType)
        {
            case EntityType_Bullet:
            case EntityType_Player:
            {
                TestEntityLineMesh = GetLineMesh(TranState->Assets, TestEntity->GameAssetID);
                if(TestEntityLineMesh)
                {
                    
                    for(u32 LineIndex = 0;
                        LineIndex < TestEntityLineMesh->LineCount;
                        ++LineIndex)
                    {
                        line Line = TestEntityLineMesh->Lines[LineIndex];
                        
                        v2 Start = PointToWorld(TestEntity, Line.Start);
                        v2 End= PointToWorld(TestEntity, Line.End);
                        DrawLine(GameBuffer, Memory,
                                 Start,
                                 End,
                                 TestEntity->Color,
                                 LineWidth);
                        
                    }
                }
            } break;
            
            
            case EntityType_Asteroid:
            {
                TestEntityLineMesh = GetLineMesh(TranState->Assets, TestEntity->GameAssetID);
                if(TestEntityLineMesh)
                {
                    
                    
                    DrawLineMeshEntity(&TempMem, GameBuffer, TestEntityLineMesh, TestEntity);
                    
                    
                }
            } break;
            
            case EntityType_Sprite:
            {
                loaded_bitmap *Bitmap = GetBitmap(TranState->Assets, GAI_A);
                PlatformCompleteAllWorkQueueWork(Memory->Queue, 0);
                if(Bitmap)
                {
                    RenderBitmap(GameBuffer, Bitmap, TestEntity->P, V2(0,0));
                }
                
            } break;
            
            case EntityType_Object:
            {
#if 0
                TestEntityLineMesh = GetLineMesh(TranState->Assets, TestEntity->GameAssetID);
                for(u32 LineIndex = 0;
                    LineIndex < TestEntityLineMesh->LineCount;
                    ++LineIndex)
                {
                    line Line = TestEntityLineMesh->Lines[LineIndex];
                    v2 Start = (TestEntity->P + Line.Start.x*TestEntity->Basis.X + Line.Start.y*TestEntity->Basis.Y);
                    v2 End = (TestEntity->P + Line.End.x*TestEntity->Basis.X + Line.End.y*TestEntity->Basis.Y);
                    DrawLine(GameBuffer, Memory,
                             Start,
                             End,
                             Line.Color,
                             LineWidth);
                }
#endif
            } break;
        }
    }
    RenderBufferToScreen(Buffer, GameBuffer);
    
    EndTemporaryMemory(TempMem);
    CheckMemory(&GameState->Arena);
    CheckMemory(&TranState->TranArena);
} 

