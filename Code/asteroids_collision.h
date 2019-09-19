#if !defined(ASTEROIDS_COLLISION_H)

internal b32
TestWall(f32 dPX, f32 dPY, f32 Wall, f32 PX, f32 PY, f32 *tMin, f32 MaxY, f32 MinY)
{
    b32 Result = false;
    if(dPX != 0)
    {
        f32 t = (Wall - PX) / dPX;
        if((t >= 0.0f) && (t < *tMin))
        {
            f32 Y = PY + t*dPY;
            if(Y < MaxY &&
               Y > MinY)
            {
                *tMin = Maximum(0.0f, t - 0.01f);
                Result = true;
            }
        }
    }   
    
    return Result;
}

internal b32 
IsFlagSet(collision_flags Source, entity_type Test)
{
    b32 Result = Source & (0x1 << (Test - 1));
    return Result;
}

internal u32
LineMeshCollisionCheck(game_state *GameState, entity *TestEntity, world_chunk *CameraChunk, game_assets *Assets)
{
    b32 Collides = false;
    u32 CollisionEntityID = 0;
    switch(TestEntity->HitboxType)
    {
        case HitboxType_LineMesh:
        {
            for(u32 HitEntityIndex = 0;
                HitEntityIndex < CameraChunk->EntityCount;
                ++HitEntityIndex)
            {
                if(TestEntity->EntityIndex != HitEntityIndex)
                {
                    entity *HitEntity = CameraChunk->Entities + HitEntityIndex;
                    if(HitEntity->Collides && 
                       IsFlagSet(TestEntity->CollisionFlags, HitEntity->EntityType))
                    {
                        line_mesh *HitEntityLineMesh = 
                            GetLineMesh(Assets, GAI_LineMeshAsteroid);
                        u32 CollideCountD = 0;
                        
                        for(u32 HitLineIndex = 0;
                            HitLineIndex < HitEntityLineMesh->LineCount;
                            ++HitLineIndex)
                        {
                            line *HitLine = HitEntityLineMesh->Lines + HitLineIndex;
                            v2 HLStart = (HitLine->Start.x*HitEntity->Basis.X +
                                          HitLine->Start.y*HitEntity->Basis.Y);
                            v2 v1 = (HitEntity->P + HLStart) - TestEntity->P;
                            b32 VertexCollision = true;
                            line_mesh *TestEntityLineMesh = 
                                GetLineMesh(Assets, GAI_LineMeshPlayer);
                            
                            for(u32 TestLineIndex = 0;
                                TestLineIndex < TestEntityLineMesh->LineCount;
                                ++TestLineIndex)
                            {
                                line *Line = TestEntityLineMesh->Lines + TestLineIndex;
                                v2 RotatedStart = Line->Start.x*TestEntity->Basis.X + Line->Start.y*TestEntity->Basis.Y;
                                
                                b32 LineCollision = true;
                                for(u32 HitLineIndex1 = 0;
                                    HitLineIndex1 < HitEntityLineMesh->LineCount;
                                    ++HitLineIndex1)
                                {
                                    line *HitLine1 = HitEntityLineMesh->Lines + HitLineIndex1;
                                    
                                    v2 RStart = (HitLine1->Start.x*HitEntity->Basis.X +
                                                 HitLine1->Start.y*HitEntity->Basis.Y);
                                    
                                    v2 REnd = (HitLine1->End.x*HitEntity->Basis.X +
                                               HitLine1->End.y*HitEntity->Basis.Y);
                                    
                                    v2 HitLine1D = REnd - RStart;
                                    v2 HitLine1DN = Normalize(HitLine1D);
                                    v2 HitLine1DPerp = Perp(HitLine1DN);
                                    v2 FromTestToHit = (TestEntity->P + RotatedStart) - (HitEntity->P + RStart);
                                    
                                    f32 VPoint = Inner(FromTestToHit, HitLine1DPerp);
                                    
                                    if(VPoint > 0)
                                    {
                                        LineCollision = false;
                                    }
                                    
                                }
                                
                                if(LineCollision)
                                {
                                    Collides = true; 
                                    CollisionEntityID = HitEntity->EntityID;
                                    TestEntity->CollisionEntityType = HitEntity->EntityType;
                                }
                                else
                                {
                                    v2 RotatedEnd = Line->End.x*TestEntity->Basis.X + Line->End.y*TestEntity->Basis.Y;
                                    
                                    v2 LineD = ((TestEntity->P + RotatedEnd) -
                                                (TestEntity->P + RotatedStart));
                                    f32 LineDLength = Length(LineD);
                                    v2 NLineD = Normalize(LineD);
                                    v2 PerpD = Perp(NLineD);
                                    
                                    f32 VPoint = Inner(v1, PerpD);
                                    VPoint -= Length(RotatedStart + LineD/2);
                                    
                                    if(VPoint > 0)
                                    {
                                        VertexCollision = false;
                                    }
                                }
                            }
                            
                            if(VertexCollision)
                            {
                                Collides = true;
                                CollisionEntityID = HitEntity->EntityID;
                                TestEntity->CollisionEntityType = HitEntity->EntityType;
                            }
                        }
                    }
                }   
            }
        } break;
    }
    
    if(!Collides)
    {
        TestEntity->CollisionEntityType = 0;
    }
    
#if 0    
    if(Collides)
    {
        TestEntity->Color = V4(1,0,0,1);
    }
    else
    {
        TestEntity->Color = V4(1,1,1,1);
    }
#endif
    
    return CollisionEntityID;
}

#define ASTEROIDS_COLLISION_H
#endif