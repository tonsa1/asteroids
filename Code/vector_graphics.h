/* date = January 23rd 2024 11:57 am */

#ifndef VECTOR_GRAPHICS_H
#define VECTOR_GRAPHICS_H

internal s32
CreateEdge(edge *Edge, v2 Start, v2 End)
{
    s32 Result = 0;
    Edge->Direction = End.y > Start.y;
    if (!Edge->Direction)
    {
        v2 Temp = Start;
        Start = End;
        End = Temp;
    }
    
    Edge->XStart = Start.x;
    Edge->YStart = Start.y;
    
    Edge->XEnd = End.x;
    Edge->YEnd = End.y;
    
    Edge->XIntersection = Start.x;
    
    Edge->Slope = (End.x - Start.x) / (End.y - Start.y);
    
    if (isinf(Edge->Slope))
    {
        Result = -1;
    }
    
    return Result;
}

internal u32
CreateEdges(v2 *Points, u32 PointCount, entity *Entity, edge *EdgeBuffer, edge **SortedEdges, u32 MaxEdgeCount, f32 Scale)
{
    u32 NewEdgeCount = 0;
    for (u32 Index = 0;
         Index < PointCount;
         ++Index)
    {
        u32 FirstIndex = Index;
        u32 NextIndex = (Index + 1) % PointCount;
        v2 Start = PointToWorld(Entity, Points[FirstIndex])*Scale;
        v2 End = PointToWorld(Entity, Points[NextIndex])*Scale;
        
        Assert(NewEdgeCount < MaxEdgeCount);
        edge *Edge = EdgeBuffer + NewEdgeCount;
        SortedEdges[NewEdgeCount++] = Edge;
        
        NewEdgeCount += CreateEdge(Edge, Start, End);
    }
    
    return NewEdgeCount;
}

internal u32
CreateEdges(line_mesh *Mesh, entity *Entity, edge *EdgeBuffer, edge **SortedEdges, u32 MaxEdgeCount, f32 Scale)
{
    
    u32 NewEdgeCount = 0;
    for (u32 Index = 0;
         Index < Mesh->LineCount;
         ++Index)
    {
        line *Line = Mesh->Lines + Index;
        
        edge *Edge = EdgeBuffer + NewEdgeCount;
        SortedEdges[NewEdgeCount++] = Edge;
        
        v2 Start = PointToWorld(Entity, Line->Start)*Scale;
        v2 End = PointToWorld(Entity, Line->End)*Scale;
        
        NewEdgeCount += CreateEdge(Edge, Start, End);
    }
    
    return NewEdgeCount;
}


void SortEdgesY(edge **Edges, u32 MaxEdgeCount)
{
    b32 IsSorted = true;
    do 
    {
        IsSorted = true;
        for (u32 Index = 0;
             Index < MaxEdgeCount - 1;
             ++Index)
        {
            u32 FirstIndex = Index;
            u32 NextIndex = Index + 1;
            
            edge *FirstEdge = Edges[FirstIndex];
            edge *NextEdge = Edges[NextIndex];
            
            if (NextEdge->YStart < FirstEdge->YStart)
            {
                IsSorted = false;
                Edges[FirstIndex] = NextEdge;
                Edges[NextIndex] = FirstEdge;
            }
        }
    } while (!IsSorted);
}

internal void
DrawMmozeiko(entity *Entity, game_buffer *GameBuffer, f32 Scale,
             v2 **Points, u32 *PointCounts, u32 PointArrayCount,
             edge *Edges, edge **SortedEdges, edge **ActiveEdges, u32 MaxEdgeCount)
{
    
    
    edge *FirstEdge = Edges;
    edge **FirstSortedEdge = SortedEdges;
    
    u32 Sum = 0;
    u32 NewCount = 0; 
    
    for(u32 Index = 0;
        Index < PointArrayCount;
        ++Index)
    {
        NewCount = CreateEdges(Points[Index], PointCounts[Index], Entity, FirstEdge, FirstSortedEdge, MaxEdgeCount, Scale);
        
        Sum += NewCount;
        FirstEdge += NewCount;
        FirstSortedEdge += NewCount;
    }
    
    SortEdgesY(SortedEdges, Sum);
    
    
    DrawShape(GameBuffer, SortedEdges, ActiveEdges, Sum);
}

internal void
CreateMmozeiko(temporary_memory *TempMem, entity *Entity, game_buffer *GameBuffer)
{
    {
        // Lower Hat
        v2 Points5[]
        {
            V2(-1.25f, -0.65f),
            V2(-0.45f, -1.0f),
            
            V2(-0.15f, -.9f),
            V2(0.15f, -.9f),
            
            V2(0.5f, -1.0f),
            V2(1.2f, -0.6f),
            
            V2(.7f, -0.5f),
            V2(-.8f, -0.55f),
        };
        
        // Upper Hat
        v2 Points6[]
        {
            V2(-0.4f, -1.15f),
            
            V2(-0.15f, -1.6f),
            
            V2(0.25f, -2.0f),
            
            V2(0.5f, -1.95f),
            V2(0.73f, -1.6f),
            V2(0.45f, -1.7f),
            
            V2(0.4f, -1.4f),
            V2(0.45f, -1.15f),
            V2(0.15f, -1.05f),
            V2(-0.2f, -1.05f),
        };
        
        v2 *Points[] =
        {
            Points5,
            Points6,
        };
        
        u32 PointCounts[] =
        {
            ArrayCount(Points5),
            ArrayCount(Points6),
        };
        
        u32 EdgeCount = ArrayCount(Points5) + ArrayCount(Points6);
        edge *Edges = PushArray(TempMem->Arena, EdgeCount, edge);
        edge **SortedEdges = PushArray(TempMem->Arena, EdgeCount, edge *);
        // TODO(Tony): this doesnt need to be full size. Instead I could just check whats the possible maximum active count
        edge **ActiveEdges = PushArray(TempMem->Arena, EdgeCount, edge *);
        
        DrawMmozeiko(Entity, GameBuffer, GameBuffer->MetersToPixels,
                     Points, PointCounts, ArrayCount(PointCounts),
                     Edges, SortedEdges, ActiveEdges, EdgeCount);
    }
    {
        
        // Head
        v2 Points1[]
        {
            V2(-0.65f, -0.65f),
            V2(0.65f, -0.65f),
            
            V2(0.65f, 1.10f),
            V2(-0.65f, 1.10f),
        };
        
        // Left Eye
        v2 Points2[]
        {
            V2(-0.45f, -0.4f),
            V2(-0.2f, -0.4f),
            V2(-0.2f, -.15f),
            V2(-0.45f, -.15f),
        };
        
        // Right Eye
        v2 Points3[]
        {
            V2(0.45f, -0.4f),
            V2(0.2f, -0.4f),
            V2(0.2f, -.15f),
            V2(0.45f, -.15f),
        };
        
        
        // Mouth
        v2 Points4[]
        {
            V2(-0.45f, 0.25f),
            V2(0.45f, 0.25f),
            V2(0.45f, .5f),
            
            V2(0.25f, .5f),
            V2(0.25f, .75f),
            V2(-0.25f, .75f),
            V2(-0.25f, .5f),
            
            
            V2(-0.45f, .5f),
        };
        
        
        
        v2 *Points[] =
        {
            Points1,
            Points2,
            Points3,
            Points4,
        };
        
        u32 PointCounts[] =
        {
            ArrayCount(Points1),
            ArrayCount(Points2),
            ArrayCount(Points3),
            ArrayCount(Points4),
        };
        
        u32 EdgeCount = ArrayCount(Points1) + ArrayCount(Points2) + ArrayCount(Points3) + ArrayCount(Points4);
        edge *Edges = PushArray(TempMem->Arena, EdgeCount, edge);
        edge **SortedEdges = PushArray(TempMem->Arena, EdgeCount, edge *);
        edge **ActiveEdges = PushArray(TempMem->Arena, EdgeCount, edge *);
        
        DrawMmozeiko(Entity, GameBuffer, GameBuffer->MetersToPixels,
                     Points, PointCounts, ArrayCount(PointCounts),
                     Edges, SortedEdges, ActiveEdges, EdgeCount);
    }
    
    
}

internal void
DrawLineMeshEntity(temporary_memory *TempMem, game_buffer *GameBuffer, line_mesh *Mesh, entity *Entity)
{
    u32 MaxEdgeCount = Mesh->LineCount;
    edge *Edges = PushArray(TempMem->Arena, MaxEdgeCount, edge);
    edge **SortedEdges = PushArray(TempMem->Arena, MaxEdgeCount, edge *);
    edge **ActiveEdges = PushArray(TempMem->Arena, MaxEdgeCount, edge *);
    
    u32 DrawCount = CreateEdges(Mesh, Entity, Edges, SortedEdges, MaxEdgeCount, GameBuffer->MetersToPixels);
    
    SortEdgesY(SortedEdges, DrawCount);
    
    DrawShape(GameBuffer, SortedEdges, ActiveEdges, DrawCount);
}

#endif //VECTOR_GRAPHICS_H
