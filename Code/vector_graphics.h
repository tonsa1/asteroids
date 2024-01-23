/* date = January 23rd 2024 11:57 am */

#ifndef VECTOR_GRAPHICS_H
#define VECTOR_GRAPHICS_H

struct edge
{
    f32 XStart;
    f32 YStart;
    
    f32 XEnd;
    f32 YEnd;
    
    f32 XIntersection;
    
    f32 Slope;
    
    // 1 = slope goes from left to right
    // 0 = slope goes from right to left
    b32 Direction;
};

void CreateEdges(v2 *Points, u32 PointCount, edge *EdgeBuffer, edge **SortedEdges, u32 MaxEdgeCount)
{
    u32 NewEdgeCount = 0;
    for (u32 Index = 0;
         Index < PointCount;
         ++Index)
    {
        Assert(NewEdgeCount < MaxEdgeCount);
        edge *Edge = EdgeBuffer + NewEdgeCount;
        SortedEdges[NewEdgeCount++] = Edge;
        
        u32 FirstIndex = Index;
        u32 NextIndex = (Index + 1) % PointCount;
        
        Edge->Direction = Points[NextIndex].y > Points[FirstIndex].y;
        
        if (!Edge->Direction)
        {
            FirstIndex = NextIndex;
            NextIndex = Index;
        }
        
        Edge->XStart = Points[FirstIndex].x;
        Edge->YStart = Points[FirstIndex].y;
        
        Edge->XEnd = Points[NextIndex].x;
        Edge->YEnd = Points[NextIndex].y;
        
        Edge->XIntersection = Points[FirstIndex].x;
        
        Edge->Slope = (Points[NextIndex].x - Points[FirstIndex].x) / (Points[NextIndex].y - Points[FirstIndex].y);
        
    }
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
            --NewEdgeCount;
        }
    }
    
    return NewEdgeCount;
}


void SortEdges(edge **SortedEdges, u32 MaxEdgeCount)
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
            
            edge *FirstEdge = SortedEdges[FirstIndex];
            edge *NextEdge = SortedEdges[NextIndex];
            
            if (NextEdge->YStart < FirstEdge->YStart)
            {
                IsSorted = false;
                SortedEdges[FirstIndex] = NextEdge;
                SortedEdges[NextIndex] = FirstEdge;
            }
        }
    } while (!IsSorted);
}

#endif //VECTOR_GRAPHICS_H
