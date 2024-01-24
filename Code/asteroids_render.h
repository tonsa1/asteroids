#if !defined(ASTEROIDS_RENDER_H)

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

void SortEdgesX(edge **Edges, u32 MaxEdgeCount)
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
            
            if (NextEdge->XIntersection < FirstEdge->XIntersection)
            {
                IsSorted = false;
                Edges[FirstIndex] = NextEdge;
                Edges[NextIndex] = FirstEdge;
            }
        }
    } while (!IsSorted);
}

internal void
ResetBufferColored(game_buffer *Buffer)
{
    u8 *Row = (u8 *)Buffer->Memory;
    for(u32 Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 XCount = 0;
        b32 thing = false;
        u32 *Pixel = (u32 *)Row;
        for(u32 X = 0;
            X < Buffer->Width;
            ++X)
        {
            if(thing)
            {
                v4 Color = {50.0f,50.0f,50.0f,1.0f};
                u32 a = (u32)Color.a;
                u32 r = (u32)Color.r;
                u32 g = (u32)Color.g;
                u32 b = (u32)Color.b;
                
                *Pixel++ = (a << 24) | (r << 16) | (g << 8) | (b << 0);      
            }
            else
            {
                v4 Color = {25.0f,75.0f,25.0f,1.0f};
                u32 a = (u32)Color.a;
                u32 r = (u32)Color.r;
                u32 g = (u32)Color.g;
                u32 b = (u32)Color.b;
                
                *Pixel++ = (a << 24) | (r << 16) | (g << 8) | (b << 0);      
            }
            ++XCount;
            if(XCount > 5)
            {
                XCount = 0;
                thing = !thing;
            }
        }
        Row += Buffer->Pitch;
    }
}

internal void
ResetBufferBlack(game_buffer *Buffer)
{
    u8 *Row = (u8 *)Buffer->Memory;
    for(u32 Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(u32 X = 0;
            X < Buffer->Width;
            ++X)
        {
            *Pixel++ = 0x00000000;
        }
        Row += Buffer->Pitch;
    }
}

internal void
RenderRect(game_buffer *Buffer, v2 P, v2 Dim, v4 Color)
{
    f32 MetersToPixel = Buffer->MetersToPixels;
    
    P *= MetersToPixel;
    Dim *= MetersToPixel;
    
    Dim /= 2;
    s32 MinX = RoundReal32ToUInt32(P.x - Dim.x);
    s32 MinY = RoundReal32ToUInt32(P.y - Dim.y);
    s32 MaxX = RoundReal32ToUInt32(P.x + Dim.x);
    s32 MaxY = RoundReal32ToUInt32(P.y + Dim.y);
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    if(MinY < 0)
    {
        MinY = 0;
    }
    if(MinX > (s32)Buffer->Width)
    {
        MinX = Buffer->Width;
    }
    if(MinY > (s32)Buffer->Height)
    {
        MinX = Buffer->Height;
    }
    if(MaxX < 0)
    {
        MaxX = 0;
    }
    if(MaxY < 0)
    {
        MaxY = 0;
    }
    if(MaxX > (s32)Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    if(MaxY > (s32)Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    
    Assert(MinX >= 0);
    Assert(MinY >= 0);
    Assert(MaxX >= 0);
    Assert(MaxY >= 0);
    
    Color.rgb *= 255;
    u8 *Row = ((u8 *)Buffer->Memory + 
               (MinX*Buffer->BytesPerPixel) +
               (MinY*Buffer->Pitch));
    for(u32 Y = MinY;
        Y < (u32)MaxY;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(u32 X = MinX;
            X < (u32)MaxX;
            ++X)
        {
            
            v4 Texel = V4((f32)((*Pixel >> 16) & 0xFF),
                          (f32)((*Pixel >> 8) & 0xFF),
                          (f32)((*Pixel >> 0) & 0xFF),
                          (f32)((*Pixel >> 24) & 0xFF));
            
            Texel =  Lerp(Texel, Color.a, Color);
            
            *Pixel++ = (((u32)(Texel.a + 0.5f) << 24) |
                        ((u32)(Texel.r + 0.5f) << 16) |
                        ((u32)(Texel.g + 0.5f) << 8) |
                        ((u32)(Texel.b + 0.5f) << 0));
        }
        Row += Buffer->Pitch;
    }
}


void 
RenderBufferToScreen(game_offscreen_buffer *Buffer, game_buffer *GameBuffer)
{
    
    u32 *SourcePixels = (u32 *)GameBuffer->Memory;
    u32 *DestPixels = (u32 *)Buffer->Memory;
    for(u32 Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        for(u32 X = 0;
            X < Buffer->Width;
            ++X)
        {
            *DestPixels++ = *SourcePixels++;
        }
    }
}

internal void
RenderBitmap(game_buffer *Buffer, loaded_bitmap *Bitmap, v2 Pos, v2 Offset)
{
    s32 MinX = RoundReal32ToUInt32(Pos.x);
    s32 MinY = RoundReal32ToUInt32(Pos.y);
    s32 MaxX = RoundReal32ToUInt32(MinX + (f32)Bitmap->Width);
    s32 MaxY = RoundReal32ToUInt32(MinY + (f32)Bitmap->Height);
    s32 Height = Bitmap->Height;
    s32 Width = Bitmap->Width;
    u32 XOffset = 0;
    u32 YOffset = 0;
    
    if(MinX < 0)
    {
        XOffset = Width;
        Width += MinX;
        XOffset -= Width;
        MinX = 0;
    }
    if(MinY < 0)
    {
        YOffset = Height;
        Height += MinY;
        YOffset -= Height;
        MinY = 0;
    }
    if(MaxX > (s32)Buffer->Width)
    {
        Width -= (MaxX - (s32)Buffer->Width);
        MaxX = (s32)Buffer->Width;
    }
    
    if(MaxY > (s32)Buffer->Height)
    {
        Height -= (MaxY - (s32)Buffer->Height);
        MaxY = (s32)Buffer->Height;
    }
    
    
    u8 *SourceRow = Bitmap->Pixels;
    SourceRow += XOffset;
    SourceRow -= (YOffset * Bitmap->Width);
    u8 *DestRow = ((u8 *)Buffer->Memory +
                   MinX*Buffer->BytesPerPixel +
                   MinY*Buffer->Pitch);
    for(s32 Y = 0;
        Y < Height;
        ++Y)
    {
        u32 *DestPixel = (u32 *)DestRow;
        u32 *SourcePixel = (u32 *)SourceRow;
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            
            f32 t = (f32)((*SourcePixel >> 24) & 0xFF) / 255.0f;
            
            f32 SR = (f32)((*SourcePixel >> 16) & 0xFF);
            f32 SG = (f32)((*SourcePixel >> 8) & 0xFF);
            f32 SB = (f32)((*SourcePixel >> 0) & 0xFF);
            
            f32 DR = (f32)((*DestPixel >> 16) & 0xFF);
            f32 DG = (f32)((*DestPixel >> 8) & 0xFF);
            f32 DB = (f32)((*DestPixel >> 0) & 0xFF);
            
            f32 A = (1.0f - t)*DR + t*SR;
            f32 R = (1.0f - t)*DR + t*SR;
            f32 G = (1.0f - t)*DG + t*SG;
            f32 B = (1.0f - t)*DB + t*SB;
            
            *DestPixel = (((u32)(R + 0.5f) << 16) |
                          ((u32)(G + 0.5f) << 8) |
                          ((u32)(B + 0.5f) << 0));
            
            ++DestPixel;
            ++SourcePixel;
        }
        
        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

#include "iacaMarks.h"

#define movdqu(A, B) A = _mm_loadu_si128((__m128i *)(B))
#define movqu(A, B) A = _mm_loadu_si64((void *)(B))
#define pshufb(A, B) A = _mm_shuffle_epi8(A, B)
#define pshufps(A, B, C, D) A = _mm256_shuffle_ps(B, C, D)
#define psrliq(A, B) _mm_srli_si128(A, B)
#define pmovzx(A, B) A = _mm256_cvtepu8_epi32(B)
#define pcvtdq2ps(A, B) A = _mm256_cvtepi32_ps(B)
#define pcvtps2dq(A, B) A = _mm256_cvtps_epi32(B)
#define pcvt2ps(A) _mm256_cvtepi32_ps(A)
#define permilps(A, B, C) A = _mm256_permute_ps(B, C);
#define perm2f128(A, B, C, D) A = _mm256_permute2f128_ps(B, C, D);
#define pblendvps(A, B, C) A = _mm256_blendv_ps(A, B, _mm256_castsi256_ps(C))
#define pfmaddps(A, B, C) _mm256_fmadd_ps(A, B, C)
#define pmulps(A, B) _mm256_mul_ps(A, B)
#define psqrtps(A) _mm256_sqrt_ps(A)
#define psubps(A, B) _mm256_sub_ps(A, B)
#define vpor(A, B) _mm256_or_si256(A, B)
#define vpxor(A, B) _mm256_xor_si256(A, B)
#define pcmpeqd(A, B) _mm256_cmpeq_epi32(A, B)
#define vpmaxps(A, B) _mm256_max_ps(A, B)
#define vpminps(A, B) _mm256_min_ps(A, B)
#define vcmpps(A, B, C) _mm256_cmp_ps(A, B, C)
#define castps_si256(A) _mm256_castps_si256(A)
#define castsi256_si128(A) _mm256_castsi256_si128(A)
#define vpaddd(A, B) _mm256_add_epi32(A, B)
#define set1d(A) _mm256_set1_epi32(A)
#define set1ps(A) _mm256_set1_ps(A)
#define setrd(...) _mm256_setr_epi32(__VA_ARGS__)
#define setb(...) _mm256_set_epi8(__VA_ARGS__)
#define setrps(...) _mm256_setr_ps(__VA_ARGS__)
#define vpslld(A, B) _mm256_slli_epi32(A, B)
#define vpsufb(A, B) _mm256_shuffle_epi8(A, B)
#define vinsertf128(A, B, C) _mm256_insertf128_si256(A, B, C)
#define vextractf128(A, B) _mm256_extractf128_si256(A, B)
#define pmaskmovd(A, B, C) _mm_maskstore_epi32(A, B, C)
#define punpcldq(A, B) _mm256_unpacklo_epi32(A, B)
#define punpchdq(A, B) _mm256_unpackhi_epi32(A, B)

char LoadShuffle[] = 
{
    3,2,1,0,7,6,5,4,
    11,10,9,8,15,14,13,12
};


void
DrawLine(game_buffer *Buffer, game_memory *Memory, v2 Start, v2 End, v4 Color, f32 LineWidth)
{
    End += 0.0001f;
    Start *= Buffer->MetersToPixels;
    End *= Buffer->MetersToPixels;
    START_COUNTER(DrawLine);
    IACA_VC64_START;
    
    Color.rgb *= 255;
    v2 d = End - Start;
    f32 VLength = Length(d);
    
    if(VLength > 0)
    {
        u8 *Pixels = (u8 *)Buffer->Memory;
        
        f32 Offset = LineWidth;
        f32 MinX = (Minimum(Start.x, End.x) - Offset);
        f32 MaxX = (Maximum(Start.x, End.x) + Offset);
        f32 MinY = (Minimum(Start.y, End.y) - Offset);
        f32 MaxY = (Maximum(Start.y, End.y) + Offset);
        
        f32 StepSize = 1.0f / VLength;
        v2 Nd = d * StepSize;
        
        if(MinX < 0)
        {
            MinX = 0;
        }
        
        if(MinY < 0)
        {
            MinY = 0;
        }
        
        if(MaxX > (s32)Buffer->Width)
        {
            MaxX = (f32)Buffer->Width;
        }
        if(MaxY > (s32)Buffer->Height)
        {
            MaxY = (f32)Buffer->Height;
        }
        
        __m256 LineWidth_x8 = set1ps(LineWidth);
        __m256 NdX = set1ps(Nd.x);
        __m256 NdY = set1ps(Nd.y);
        __m256 StartX_x8 = set1ps(Start.x);
        __m256 StartY_x8 = set1ps(Start.y);
        __m256 EndX_x8 = set1ps(End.x);
        __m256 EndY_x8 = set1ps(End.y);
        __m256 Color_x8 = setrps(Color.b, Color.g, Color.r, Color.a, Color.b, Color.g, Color.r, Color.a);
        __m256 Ones = set1ps(1.0f);
        __m256 MinusOnes = set1ps(-1.0f);
        __m256 Zeroes = set1ps(0.0f);
        __m256i Zeroesi = set1d(0);
        __m256 t = set1ps(Color.a); 
        __m256 VLength_x8 = set1ps(VLength); 
        __m256i XAdd = setrd(0,1,2,3,0,1,2,3);
        __m256i YAdd = setrd(0,0,0,0,1,1,1,1);
        __m256 OneMinusT = psubps(Ones, t);
        __m256 PerpX = set1ps(-Nd.y);
        __m256 PerpY = set1ps(Nd.x);
        __m256 MX = set1ps(MaxX);
        for(s32 X = (s32)MinX;
            X < MaxX;
            X += 4)
        {
            
            for(s32 Y = (s32)MinY;
                Y < MaxY;
                Y += 2)
            {
                START_COUNTER(PerPixel);
                
                __m256 PixelDistance = Ones;
                int *Ptr0 = (int *)((u8 *)Pixels + (u32)(X)*sizeof(u32) + (u32)(Y)*Buffer->Pitch);
                int *Ptr1 = (int *)((u8 *)Ptr0 + Buffer->Pitch);
                __m256i PixelPos = setrd(0,1,2,3,
                                         0+Buffer->Width,
                                         1+Buffer->Width,
                                         2+Buffer->Width,
                                         3+Buffer->Width);
                __m256 PixelPX, PixelPY;
                pcvtdq2ps(PixelPX, vpaddd(set1d(X), XAdd));
                pcvtdq2ps(PixelPY, vpaddd(set1d(Y), YAdd));
                __m256i WriteMask = castps_si256(vcmpps(psubps(MX, PixelPX), Zeroes, _CMP_GT_OQ));
                
                
                __m128i FirstRow, SecondRow;
                __m128i FirstRowHigh, SecondRowHigh;
                __m128i Shuffle;
                
                movdqu(Shuffle, LoadShuffle);
                
                // load pixels
                movdqu(FirstRow, Ptr0);
                movdqu(SecondRow, Ptr1);
                
                // Shift upper half to lower
                FirstRowHigh = psrliq(FirstRow, 8);
                SecondRowHigh = psrliq(SecondRow, 8);
                
                
                __m256i Texel0_1, Texel2_3, Texel4_5, Texel6_7;
                __m256 Texel01, Texel23, Texel45, Texel67;
                // Zero extend 
                pmovzx(Texel0_1, FirstRow);
                pmovzx(Texel2_3, FirstRowHigh);
                pmovzx(Texel4_5, SecondRow);
                pmovzx(Texel6_7, SecondRowHigh);
                
                // convert to float
                pcvtdq2ps(Texel01, Texel0_1);
                pcvtdq2ps(Texel23, Texel2_3);
                pcvtdq2ps(Texel45, Texel4_5);
                pcvtdq2ps(Texel67, Texel6_7);
                
                __m256 TestPixeldX = psubps(PixelPX, StartX_x8);
                __m256 TestPixeldY = psubps(PixelPY, StartY_x8);
                
                __m256 Point = pfmaddps(TestPixeldY, NdY, pmulps(TestPixeldX, NdX));
                
                // PointLength < 0 //
                __m256i TotalMask = set1d(0);
                TotalMask = castps_si256(vcmpps(Point, Zeroes, _CMP_LT_OQ));
                
                __m256 NewPixelDistance = psqrtps(pfmaddps(TestPixeldX, TestPixeldX, pmulps(TestPixeldY,TestPixeldY)));
                
                pblendvps(PixelDistance, NewPixelDistance, TotalMask);
                
                // PointLength > VLength //
                __m256i Mask = castps_si256(vcmpps(Point, VLength_x8, _CMP_GT_OQ));
                
                TotalMask = vpor(TotalMask, Mask);
                
                __m256 TestLengthX = psubps(PixelPX, EndX_x8);
                __m256 TestLengthY = psubps(PixelPY, EndY_x8);
                NewPixelDistance = psqrtps(pfmaddps(TestLengthX, TestLengthX, pmulps(TestLengthY,TestLengthY)));
                
                pblendvps(PixelDistance, NewPixelDistance, Mask);
                
                NewPixelDistance = pfmaddps(TestPixeldX,PerpX, pmulps(TestPixeldY, PerpY));
                NewPixelDistance = vpmaxps(NewPixelDistance, pmulps(NewPixelDistance, MinusOnes));
                
                TotalMask = vpxor(TotalMask, pcmpeqd(Zeroesi, Zeroesi));
                pblendvps(PixelDistance, NewPixelDistance, TotalMask);
                
                
                PixelDistance = psubps(PixelDistance, psubps(LineWidth_x8, Ones));
                PixelDistance = vpmaxps(PixelDistance, Zeroes);
                PixelDistance = pmulps(PixelDistance,PixelDistance);
                PixelDistance = vpminps(PixelDistance, Ones);
                
                __m256 t01, t23, t45, t67;
                __m256 pd01, pd23, pd45, pd67;
                __m256 Shuffle0_4, Shuffle1_5, Shuffle2_6, Shuffle3_7;
                permilps(Shuffle0_4, PixelDistance, 0);
                permilps(Shuffle1_5, PixelDistance, 0b01010101);
                permilps(Shuffle2_6, PixelDistance, 0b10101010);
                permilps(Shuffle3_7, PixelDistance, 0b11111111);
                
                perm2f128(pd01, Shuffle0_4, Shuffle1_5, 0b100000);
                perm2f128(pd23, Shuffle2_6, Shuffle3_7, 0b100000);
                perm2f128(pd45, Shuffle0_4, Shuffle1_5, 0b110001);
                perm2f128(pd67, Shuffle2_6, Shuffle3_7, 0b110001);
                
                t01 = psubps(Ones, pd01);
                t23 = psubps(Ones, pd23);
                t45 = psubps(Ones, pd45);
                t67 = psubps(Ones, pd67);
                
                Texel01 = pfmaddps(t01, pfmaddps(OneMinusT, Texel01, pmulps(t, Color_x8)), pmulps(pd01, Texel01));
                Texel23 = pfmaddps(t23, pfmaddps(OneMinusT, Texel23, pmulps(t, Color_x8)), pmulps(pd23, Texel23));
                Texel45 = pfmaddps(t45, pfmaddps(OneMinusT, Texel45, pmulps(t, Color_x8)), pmulps(pd45, Texel45));
                Texel67 = pfmaddps(t67, pfmaddps(OneMinusT, Texel67, pmulps(t, Color_x8)), pmulps(pd67, Texel67));
                
                
                
                __m256i a01, a23, a45, a67;
                pcvtps2dq(a01, Texel01);
                pcvtps2dq(a23, Texel23);
                pcvtps2dq(a45, Texel45);
                pcvtps2dq(a67, Texel67);
                
                
                __m256i shift0 = vpslld(a01, 0);
                __m256i shift1 = vpslld(a23, 8);
                __m256i shift2 = vpslld(a45, 16);
                __m256i shift3 = vpslld(a67, 24);
                shift0 = vpor(shift3, vpor(shift2, vpor(shift1, shift0)));
                
                
                __m256i shiftTest = vpsufb(shift0, setb(31,27,23,19,
                                                        30,26,22,18,
                                                        29,25,21,17,
                                                        28,24,20,16,
                                                        15,11,7,3,
                                                        14,10,6,2,
                                                        13,9,5,1,
                                                        12,8,4,0));
                
                __m256i temp0 = {};
                __m256i temp1 = {};
                __m256i temp2 = {};
                
                temp0 = vinsertf128(temp0, vextractf128(shiftTest, 1),0);
                temp1 = punpcldq(shiftTest, temp0);
                temp2 = punpchdq(shiftTest, temp0);
                
                pmaskmovd(Ptr0, vextractf128(WriteMask, 0), castsi256_si128(temp1));
                pmaskmovd(Ptr1, vextractf128(WriteMask, 0), castsi256_si128(temp2));
                
                END_COUNTER(PerPixel);
            }
        }
    }
    
    IACA_VC64_END;
    END_COUNTER(DrawLine);
}

void
DrawLineSlowly(game_buffer *Buffer, game_memory *Memory, v2 Start, v2 End, v4 Color, f32 LineWidth)
{
    START_COUNTER(DrawLine);
    
    Start *= Buffer->MetersToPixels;
    End *= Buffer->MetersToPixels;
    
    Color.rgb *= 255;
    v2 d = End - Start;
    
    f32 VLength = Length(d);
    
    if(VLength > 0)
    {
        f32 XCount = Absolute(End.x - Start.x);
        f32 YCount = Absolute(End.y - Start.y);
        u8 *Pixels = (u8 *)Buffer->Memory;
        
#if 1
        f32 Width = LineWidth * 4.0f;
        f32 MinX = (Minimum(Start.x, End.x) - Width);
        f32 MaxX = (Maximum(Start.x, End.x) + Width);
        f32 MinY = (Minimum(Start.y, End.y)  - Width);
        f32 MaxY = (Maximum(Start.y, End.y) + Width);
#else
        f32 MinX = (Minimum(Start.x, End.x));
        f32 MaxX = (Maximum(Start.x, End.x));
        f32 MinY = (Minimum(Start.y, End.y));
        f32 MaxY = (Maximum(Start.y, End.y));
#endif
        f32 StepSize = 1.0f / VLength;
        v2 Nd = d * StepSize;
        
        if(MinX < 0)
        {
            MinX = 0;
        }
        
        if(MinY < 0)
        {
            MinY = 0;
        }
        
        if(MaxX > (s32)Buffer->Width)
        {
            MaxX = (f32)Buffer->Width;
        }
        if(MaxY > (s32)Buffer->Height)
        {
            MaxY = (f32)Buffer->Height;
        }
        
        v2 PerpD = Perp(Nd);
        
        for(s32 X = (s32)MinX;
            X < (s32)MaxX;
            ++X)
        {
            for(s32 Y = (s32)MinY;
                Y < MaxY;
                ++Y)
            {
                START_COUNTER(PerPixel)
                    
                    v2 PixelP = V2i(X, Y);
                u32 *PixelPtrA = (u32 *)((u8 *)Pixels + 
                                         (u32)PixelP.x*sizeof(u32) +
                                         (u32)PixelP.y*Buffer->Pitch);
                v4 TexelA = V4((f32)((*PixelPtrA >> 16) & 0xFF),
                               (f32)((*PixelPtrA >> 8) & 0xFF),
                               (f32)((*PixelPtrA >> 0) & 0xFF),
                               (f32)((*PixelPtrA >> 24) & 0xFF));
                
                v2 TestPixeld = PixelP - Start;
                f32 PixelDistance = 1;
                f32 Point = Inner(TestPixeld, Nd);
                
                if (Point < 0)
                {
                    PixelDistance = Length(TestPixeld);
                }
                else if(Point > VLength)
                {
                    v2 TestLength = PixelP - End;
                    PixelDistance = Length(TestLength);
                }
                else
                {
                    PixelDistance = Inner(TestPixeld, PerpD);
                    PixelDistance = Absolute(PixelDistance);
                }
                
                
                
                PixelDistance -= LineWidth - 1.0f;
                
                
                
                
                if(PixelDistance < 0)
                    PixelDistance = 0;
                
                PixelDistance *= (PixelDistance);
                
                if(PixelDistance > 1)
                    PixelDistance = 1;
                
                
                TexelA = Lerp(Lerp(TexelA, Color.a, Color), PixelDistance, TexelA);
                
                *PixelPtrA = (((u32)(TexelA.a + 0.5f) << 24) |
                              ((u32)(TexelA.r + 0.5f) << 16) |
                              ((u32)(TexelA.g + 0.5f) << 8) |
                              ((u32)(TexelA.b + 0.5f) << 0));
                END_COUNTER(PerPixel)
            }
        }
        
    }
    END_COUNTER(DrawLine)
}

void
DrawBezier(game_buffer *Buffer, game_memory *Memory)
{
    v2 Start = V2(0,0);
    v2 End = V2(4,4);
    
    v2 A = Start*Buffer->MetersToPixels;
    v2 B = V2(2,4)*Buffer->MetersToPixels;
    //v2 C = V2(2,2)*Buffer->MetersToPixels;
    v2 C = V2(4,0)*Buffer->MetersToPixels;
    
    Start *= Buffer->MetersToPixels;
    End *= Buffer->MetersToPixels;
    
    v2 d = Normalize(C - A);
    v2 n = Perp(d);
    f32 VLength = Length(C - A);
    f32 InvVLength = 1.0f / VLength;
    
    if(VLength > 0)
    {
        f32 XCount = Absolute(End.x - Start.x);
        f32 YCount = Absolute(End.y - Start.y);
        u8 *Pixels = (u8 *)Buffer->Memory;
        
        f32 MinX = (Minimum(Start.x, End.x));
        f32 MaxX = (Maximum(Start.x, End.x));
        f32 MinY = (Minimum(Start.y, End.y));
        f32 MaxY = (Maximum(Start.y, End.y));
        
        if(MinX < 0)
        {
            MinX = 0;
        }
        
        if(MinY < 0)
        {
            MinY = 0;
        }
        
        if(MaxX > (s32)Buffer->Width)
        {
            MaxX = (f32)Buffer->Width;
        }
        
        if(MaxY > (s32)Buffer->Height)
        {
            MaxY = (f32)Buffer->Height;
        }
        
        
        for(s32 X = (s32)MinX;
            X < (s32)MaxX;
            ++X)
        {
            for(s32 Y = (s32)MinY;
                Y < MaxY;
                ++Y)
            {
                v2 PixelP = V2i(X, Y);
                v2 Pixeld = PixelP - Start;
                
                f32 t = Inner(d,Pixeld)*InvVLength;
                v2 ddotPixelP = Lerp(A,t,C);
                
                v2 AB = Lerp(A,t,B);
                v2 BC = Lerp(B,t,C);
                v2 Tangent = Normalize(BC - AB);
                
                v2 ABBC = Lerp(AB,t,BC);
                v2 BezierPointToPixel = PixelP - ABBC;
                f32 PixelDistance = Absolute(Inner(BezierPointToPixel,  Perp(Tangent)));
                
                if(PixelDistance < 0)
                {
                    PixelDistance = 0;
                }
                f32 Maxx = 5.f;
                if(PixelDistance > Maxx)
                {
                    PixelDistance = Maxx;
                }
                
                v4 Color = V4(255,255,255,255);
                u32 *PixelPtrA = (u32 *)((u8 *)Pixels + 
                                         (u32)PixelP.x*sizeof(u32) +
                                         (u32)PixelP.y*Buffer->Pitch);
                v4 TexelA = V4((f32)((*PixelPtrA >> 16) & 0xFF),
                               (f32)((*PixelPtrA >> 8) & 0xFF),
                               (f32)((*PixelPtrA >> 0) & 0xFF),
                               (f32)((*PixelPtrA >> 24) & 0xFF));
                
                TexelA = Lerp(Color, PixelDistance/Maxx, TexelA);
                
                *PixelPtrA = (((u32)(TexelA.a + 0.5f) << 24) |
                              ((u32)(TexelA.r + 0.5f) << 16) |
                              ((u32)(TexelA.g + 0.5f) << 8) |
                              ((u32)(TexelA.b + 0.5f) << 0));
            }
            
        }
    }
}

void
DrawShape(game_buffer *Buffer, edge **SortedEdges, edge **ActiveEdges, u32 SortedCount)
{
    s32 YStart = (u32)SortedEdges[0]->YStart;
    YStart = YStart < 0 ? 0 : YStart;
    
    u32 FirstActiveIndex = 0;
    s32 ActiveCount = 0;
    
    // ARGB
    u32 Color = 0xFFFFFFFF;
    
    u8 A8 = (Color >> 24) & 0xFF;
    u8 R8 = (Color >> 16) & 0xFF;
    u8 G8 = (Color >> 8) & 0xFF;
    u8 B8 = Color & 0xFF;
    
    f32 A = (f32)A8 / 255.f;
    f32 R = (f32)R8 / 255.f;
    f32 G = (f32)G8 / 255.f;
    f32 B = (f32)B8 / 255.f;
    
    const u32 AlphaStepCount = 5;
    f32 Offsets[AlphaStepCount] = 
    {
        0.1f,
        0.3f,
        0.5f,
        0.7f,
        0.9f,
    };
    f32 AlphaPerStep = (1.0f / (f32)AlphaStepCount);
    // TODO(Tony): change this buffer size to screen width
    u8 AlphaBuffer[1024] = {};
    
    u8 *Pixels = (u8 *)Buffer->Memory;
    for(u32 Y = YStart;
        Y < Buffer->Height;
        ++Y)
    {
        
        s32 AlphaStart = 9999;
        s32 AlphaEnd = 0;
        
        u32 *PixelRow = (u32 *)(Pixels + (Y * Buffer->Pitch));
        
        u32 Skip = true;
        for (u32 StepIndex = 0;
             StepIndex < AlphaStepCount;
             ++StepIndex)
        {
            f32 CurrentY = (f32)Y + Offsets[StepIndex];
            
            //calculate current X intersection and remove lines that are not active anymore
            {
                u32 RemoveCount = 0;
                for (s32 ActiveIndex = 0;
                     ActiveIndex < ActiveCount;
                     ++ActiveIndex)
                {
                    edge *ActiveEdge = ActiveEdges[ActiveIndex];
                    
                    if (CurrentY > ActiveEdge->YEnd)
                    {
                        ActiveEdges[ActiveIndex] = 0;
                        ++RemoveCount;
                    }
                    else
                    {
                        f32 Diff = CurrentY - ActiveEdge->YStart;
                        ActiveEdge->XIntersection = ActiveEdge->XStart + ActiveEdge->Slope * Diff;
                        
                        if (RemoveCount)
                        {
                            ActiveEdges[ActiveIndex - RemoveCount] = ActiveEdge;
                        }
                    }
                }
                
                ActiveCount -= RemoveCount;
            }
            
            // add edges that are now active
            {
                for (u32 Index = FirstActiveIndex;
                     Index < SortedCount;
                     ++Index)
                {
                    edge *Edge = SortedEdges[Index];
                    
                    if (CurrentY > Edge->YStart)
                    {
                        ++FirstActiveIndex;
                        if (CurrentY <= Edge->YEnd)
                        {
                            f32 Diff = CurrentY - Edge->YStart;
                            Edge->XIntersection += Edge->Slope * Diff;
                            ActiveEdges[ActiveCount++] = Edge;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            
            // write alpha values
            if (ActiveCount)
            {
                Skip = false;
                // There should never be an uneven amount of lines, I think?
                Assert((ActiveCount % 2) == 0);
                
                // TODO(Tony): I'm sure some other type of sorting is much better here. But thats only relevant for optimization phase.
                SortEdgesX(ActiveEdges, ActiveCount);
                
                // TODO(Tony): Only supports OddEven fill at the moment.
                // TODO(Tony): Add WindingFill
                for(s32 ActiveIndex = 0;
                    ActiveIndex < ActiveCount - 1;
                    ActiveIndex += 2)
                {
                    f32 FStart = ActiveEdges[ActiveIndex]->XIntersection;
                    f32 FEnd = ActiveEdges[ActiveIndex + 1]->XIntersection;
                    
                    FStart = FStart >= 0.f ? FStart : 0.f;
                    FEnd = FEnd < (f32)Buffer->Width ? FEnd : (f32)Buffer->Width - 1;
                    
                    s32 Start = (s32)FStart;
                    s32 End = (s32)FEnd;
                    
                    AlphaStart = Start < AlphaStart ? Start : AlphaStart;
                    AlphaEnd = End > AlphaEnd ? End : AlphaEnd;
                    
                    // 0 - Alpha for the first pixel
                    // 1 - Alpha for all the pixels between first and last
                    // 2 - Alpha for the last pixel
                    f32 Alphas[3] =
                    {
                        (1.0f - (FStart - (f32)Start)),
                        A,
                        FEnd - (f32)End,
                    };
                    
                    s32 IndexDivisor = 0;
                    s32 TestIndex = 0;
                    
                    // Check if were only writing 1 or 2 alphas
                    {
                        s32 FillDist = (End - Start) + 1;
                        if (FillDist == 1)
                        {
                            f32 RemainingAlpha = Alphas[2] - (1.0f - Alphas[0]);
                            Alphas[1] = RemainingAlpha;
                            IndexDivisor = 1;
                            TestIndex = 1;
                            
                        }
                        else if (FillDist == 2)
                        {
                            Alphas[1] = Alphas[2] * A;
                            IndexDivisor = 1;
                        }
                        else
                        {
                            IndexDivisor = FillDist - 2;
                            TestIndex = IndexDivisor - 1;
                        }
                        
                        Alphas[0] *= A;
                        Alphas[2] *= A;
                    }
                    
#if 0
                    
                    Alphas[0] *= Alphas[0];
                    Alphas[1] *= Alphas[1];
                    Alphas[2] *= Alphas[2];
#endif
                    
                    u8 Alphas8[3] = 
                    {
                        (u8)(Alphas[0] * AlphaPerStep * 255.f),
                        (u8)(Alphas[1] * AlphaPerStep * 255.f),
                        (u8)(Alphas[2] * AlphaPerStep * 255.f),
                    };
                    
                    // Blend and write alphas
                    for (s32 BufferIndex = Start;
                         BufferIndex <= End;
                         ++BufferIndex)
                    {
                        s32 AlphaIndex = TestIndex++ / IndexDivisor;
                        
                        AlphaBuffer[BufferIndex] += Alphas8[AlphaIndex]; 
                    }
                }
            }
        }
        
        // skip if no edges are active.
        if (!Skip)
        {
            // Blend and write pixels
            for (s32 RowIndex = AlphaStart;
                 RowIndex <= AlphaEnd;
                 ++RowIndex)
            {
                u32 *PixelPtrA = PixelRow + RowIndex;
                
                u32 PixelColor = *PixelPtrA;
                f32 PR = ((PixelColor >> 16) & 0xFF) / 255.f;
                f32 PG = ((PixelColor >> 8) & 0xFF) / 255.f;
                f32 PB = (PixelColor & 0xFF) / 255.f;
                
                f32 Alpha = ((f32)AlphaBuffer[RowIndex]) / 255.f;
                AlphaBuffer[RowIndex] = 0;
                
                f32 NewR = Lerp(PR, Alpha, R);
                f32 NewG = Lerp(PG, Alpha, G);
                f32 NewB = Lerp(PB, Alpha, B);
                
                u32 NewColor = ((((u32)(NewR * 255.f) & 0xFF) << 16) |
                                (((u32)(NewG * 255.f) & 0xFF) << 8)  |
                                ((u32)(NewB * 255.f) & 0xFF));
                
                *PixelPtrA = NewColor;
            }
        }
        
        //end if no edges are left
        if (FirstActiveIndex == SortedCount && ActiveCount == 0)
        {
            break;
        }
    }
}

#define ASTEROIDS_RENDER_H
#endif