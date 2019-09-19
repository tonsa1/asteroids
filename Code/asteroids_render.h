#if !defined(ASTEROIDS_RENDER_H)

#pragma pack(push,1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

struct bit_scan_result
{
    b32 Found;
    u32 Index;
};

bit_scan_result
FindLeastSignificantBit(u32 Bit)
{
    bit_scan_result Result = {};
    
    for(u32 Index = 0;
        Index < 32;
        ++Index)
    {
        if((1 << Index) & Bit)
        {
            Result.Found = true;
            Result.Index = Index;
            break;
        }
    }
    
    return Result;
}

internal loaded_bitmap
DEBUGLoadBMP(debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
    loaded_bitmap Bitmap = {};
    
    debug_read_file_result ReadResult = ReadEntireFile(Filename);
    if(ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        Bitmap.Pixels = (u8 *)ReadResult.Contents + Header->BitmapOffset;
        Bitmap.Width = Header->Width;
        Bitmap.Height = Header->Height;   
        
        u32 RedMask = Header->RedMask;
        u32 GreenMask = Header->GreenMask;
        u32 BlueMask = Header->BlueMask;
        u32 AlphaMask = ~(RedMask|GreenMask|BlueMask);
        
        bit_scan_result AlphaScan = FindLeastSignificantBit(AlphaMask);
        bit_scan_result RedScan = FindLeastSignificantBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantBit(BlueMask);
        
        u32 *Pixels = (u32 *)Bitmap.Pixels;
        for(u32 Y = 0;
            Y < (u32)Bitmap.Height;
            ++Y)
        {
            for(u32 X = 0;
                X < (u32)Bitmap.Width;
                ++X)
            {
                u32 NewPixel = *Pixels;
                
                f32 R = (f32)((NewPixel & RedMask) >> RedScan.Index);
                f32 G = (f32)((NewPixel & GreenMask) >>GreenScan.Index);
                f32 B = (f32)((NewPixel & BlueMask) >> BlueScan.Index);
                f32 A = (f32)((NewPixel & AlphaMask) >> AlphaScan.Index);
                f32 AN = (A / 255.0f);
                
                R = R*AN;
                G = G*AN;
                B = B*AN;
                
                *Pixels++ = (((u32)(A + 0.5f) << 24) |
                             ((u32)(R + 0.5f) << 16) |
                             ((u32)(G + 0.5f) << 8) |
                             ((u32)(B + 0.5f) << 0));
                
                
                
            }
        }
    }
    Bitmap.Pitch = -Bitmap.Width*sizeof(u32);
    Bitmap.Pixels = (u8 *)Bitmap.Pixels - Bitmap.Pitch*(Bitmap.Height - 1); 
    return Bitmap;
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

internal void
DrawRectangleSlowly(game_buffer *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                    loaded_bitmap *Texture)
{
    Origin *= Buffer->MetersToPixels;
    YAxis *= Buffer->MetersToPixels;
    XAxis *= Buffer->MetersToPixels;
    f32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
    f32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);
    
    u32 Color32 = ((RoundReal32ToUInt32(Color.a * 255.0f) << 24) |
                   (RoundReal32ToUInt32(Color.r * 255.0f) << 16) |
                   (RoundReal32ToUInt32(Color.g * 255.0f) << 8) |
                   (RoundReal32ToUInt32(Color.b * 255.0f) << 0));
    
    int WidthMax = (Buffer->Width - 1);
    int HeightMax = (Buffer->Height - 1);
    
    int XMin = WidthMax;
    int XMax = 0;
    int YMin = HeightMax;
    int YMax = 0;
    
    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};
    for(int PIndex = 0;
        PIndex < ArrayCount(P);
        ++PIndex)
    {
        v2 TestP = P[PIndex];
        int FloorX = FloorReal32ToInt32(TestP.x);
        int CeilX = CeilReal32ToInt32(TestP.x);
        int FloorY = FloorReal32ToInt32(TestP.y);
        int CeilY = CeilReal32ToInt32(TestP.y);
        
        if(XMin > FloorX) {XMin = FloorX;}
        if(YMin > FloorY) {YMin = FloorY;}
        if(XMax < CeilX) {XMax = CeilX;}
        if(YMax < CeilY) {YMax = CeilY;}
    }
    
    if(XMin < 0) {XMin = 0;}
    if(YMin < 0) {YMin = 0;}
    if(XMax > WidthMax) {XMax = WidthMax;}
    if(YMax > HeightMax) {YMax = HeightMax;}    
    
    u8 *Row = ((u8 *)Buffer->Memory +
               XMin*BITMAP_BYTES_PER_PIXEL +
               YMin*Buffer->Pitch);
    for(int Y = YMin;
        Y <= YMax;
        ++Y)
    {
        
        u32 *Pixel = (u32 *)Row;
        for(int X = XMin;
            X <= XMax;
            ++X)
        {
#if 1
            v2 PixelP = V2i(X, Y);
            v2 d = PixelP - Origin;
            
            // TODO(casey): PerpInner
            // TODO(casey): Simpler origin
            f32 Edge0 = Inner(d, -Perp(XAxis));
            f32 Edge1 = Inner(d - XAxis, -Perp(YAxis));
            f32 Edge2 = Inner(d - XAxis - YAxis, Perp(XAxis));
            f32 Edge3 = Inner(d - YAxis, Perp(YAxis));
            
            if((Edge0 < 0) &&
               (Edge1 < 0) &&
               (Edge2 < 0) &&
               (Edge3 < 0))
            {
                f32 U = InvXAxisLengthSq*Inner(d, XAxis);
                f32 V = InvYAxisLengthSq*Inner(d, YAxis);
                
                // TODO(casey): SSE clamping.
                Assert((U >= 0.0f) && (U <= 1.0f));
                Assert((V >= 0.0f) && (V <= 1.0f));
                
                // TODO(casey): Formalize texture boundaries!!!
                f32 tX = ((U*(f32)(Texture->Width - 2)));
                f32 tY = ((V*(f32)(Texture->Height - 2)));
                
                s32 X = (s32)tX;
                s32 Y = (s32)tY;
                
                f32 fX = tX - (f32)X;
                f32 fY = tY - (f32)Y;
                
                Assert((X >= 0) && (X < Texture->Width));
                Assert((Y >= 0) && (Y < Texture->Height));
                
                u8 *TexelPtr = ((u8 *)Texture->Pixels) + Y*Texture->Pitch + X*sizeof(u32);
                u32 TexelPtrA = *(u32 *)(TexelPtr);
                u32 TexelPtrB = *(u32 *)(TexelPtr + sizeof(u32));
                u32 TexelPtrC = *(u32 *)(TexelPtr + Texture->Pitch);
                u32 TexelPtrD = *(u32 *)(TexelPtr + Texture->Pitch + sizeof(u32));
                
                // TODO(casey): Color.a!!
                v4 TexelA = {(f32)((TexelPtrA >> 16) & 0xFF),
                    (f32)((TexelPtrA >> 8) & 0xFF),
                    (f32)((TexelPtrA >> 0) & 0xFF),
                    (f32)((TexelPtrA >> 24) & 0xFF)};
                v4 TexelB = {(f32)((TexelPtrB >> 16) & 0xFF),
                    (f32)((TexelPtrB >> 8) & 0xFF),
                    (f32)((TexelPtrB >> 0) & 0xFF),
                    (f32)((TexelPtrB >> 24) & 0xFF)};
                v4 TexelC = {(f32)((TexelPtrC >> 16) & 0xFF),
                    (f32)((TexelPtrC >> 8) & 0xFF),
                    (f32)((TexelPtrC >> 0) & 0xFF),
                    (f32)((TexelPtrC >> 24) & 0xFF)};
                v4 TexelD = {(f32)((TexelPtrD >> 16) & 0xFF),
                    (f32)((TexelPtrD >> 8) & 0xFF),
                    (f32)((TexelPtrD >> 0) & 0xFF),
                    (f32)((TexelPtrD >> 24) & 0xFF)};
                
                TexelA = SRGB255ToLinear1(TexelA);
                TexelB = SRGB255ToLinear1(TexelB);
                TexelC = SRGB255ToLinear1(TexelC);
                TexelD = SRGB255ToLinear1(TexelD);
#if 1
                v4 Texel = Lerp(Lerp(TexelA, fX, TexelB),
                                fY,
                                Lerp(TexelC, fX, TexelD));
#else
                v4 Texel = TexelA;
#endif
                f32 RSA = Texel.a * Color.a; 
                
                v4 Dest = {(f32)((*Pixel >> 16) & 0xFF),
                    (f32)((*Pixel >> 8) & 0xFF),
                    (f32)((*Pixel >> 0) & 0xFF),
                    (f32)((*Pixel >> 24) & 0xFF)};
                
                Dest = SRGB255ToLinear1(Dest);
                
                f32 RDA = Dest.a;
                
                f32 InvRSA = (1.0f-RSA);
                
                v4 DestColor = {InvRSA*Dest.r + Texel.a*Color.a*Color.r*Texel.r,
                    InvRSA*Dest.g + Texel.a*Color.a*Color.g*Texel.g,
                    InvRSA*Dest.b + Texel.a*Color.a*Color.b*Texel.b,
                    (RSA + RDA - RSA*RDA)};
                
                DestColor = Linear1ToSRGB255(DestColor);
                
                *Pixel = (((u32)(DestColor.a + 0.5f) << 24) |
                          ((u32)(DestColor.r + 0.5f) << 16) |
                          ((u32)(DestColor.g + 0.5f) << 8) |
                          ((u32)(DestColor.b + 0.5f) << 0));
            }
#else
            *Pixel = Color32;
#endif
            
            ++Pixel;
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
        
        
#define M(Array, I) ((f32 *)&(Array))[I]
#define Mi(Array, I) ((u32 *)&(Array))[I]
#define Pi(Array, I) (u32 *)(((uint64 *)&(Array))[I])
#define GetPointer(X,Y) (u32 *)((u8 *)Pixels + (u32)(X)*sizeof(u32) + (u32)(Y)*Buffer->Pitch);
#define Lo4(m) *((__m128 *)&m)
#define Hi4(m) *(((__m128 *)&m) + 1)
#define Lo4i(m) *((__m128i *)&m)
#define Hi4i(m) *(((__m128i *)&m) + 1)
        
        __m256 LineWidth_x8 = _mm256_set1_ps(LineWidth);
        __m256 NdX = _mm256_set1_ps(Nd.x);
        __m256 NdY = _mm256_set1_ps(Nd.y);
        __m256 StartX_x8 = _mm256_set1_ps(Start.x);
        __m256 StartY_x8 = _mm256_set1_ps(Start.y);
        __m256 EndX_x8 = _mm256_set1_ps(End.x);
        __m256 EndY_x8 = _mm256_set1_ps(End.y);
        __m256 Color_x8 = _mm256_setr_ps(Color.a, Color.r, Color.g, Color.b, Color.a, Color.r, Color.g, Color.b);
        __m256 Ones = _mm256_set1_ps(1.0f);
        __m256 MinusOnes = _mm256_set1_ps(-1.0f);
        __m256 Zeroes = _mm256_set1_ps(0.0f);
        __m256 t = _mm256_set1_ps(Color.a); 
        __m256 VLength_x8 = _mm256_set1_ps(VLength); 
        __m256i XAdd = _mm256_setr_epi32(0,1,2,3,0,1,2,3);
        __m256i YAdd = _mm256_setr_epi32(0,0,0,0,1,1,1,1);
        __m256 OneMinusT = _mm256_sub_ps(Ones, t);
        __m256 PerpX = _mm256_set1_ps(-Nd.y);
        __m256 PerpY = _mm256_set1_ps(Nd.x);
        __m256 MX = _mm256_set1_ps(MaxX);
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
                __m256i PixelPos = _mm256_setr_epi32(0,1,2,3,
                                                     0+Buffer->Width,
                                                     1+Buffer->Width,
                                                     2+Buffer->Width,
                                                     3+Buffer->Width);
                
                __m256i PixelPX = _mm256_add_epi32(_mm256_set1_epi32(X), XAdd);
                __m256i PixelPY = _mm256_add_epi32(_mm256_set1_epi32(Y), YAdd);
                __m256i WriteMask = _mm256_castps_si256(_mm256_cmp_ps(_mm256_sub_ps(MX, _mm256_cvtepi32_ps(PixelPX)), Zeroes, _CMP_GT_OQ));
                __m256i PointerPixels = _mm256_i32gather_epi32(Ptr0, PixelPos, 4);
                __m256i asd = _mm256_set1_epi32(Mi(PointerPixels,3));
                
                u32 PixelPtr0 = _mm256_extract_epi32(PointerPixels, 0);
                u32 PixelPtr1 = _mm256_extract_epi32(PointerPixels, 1);
                u32 PixelPtr2 = _mm256_extract_epi32(PointerPixels, 2);
                u32 PixelPtr3 = _mm256_extract_epi32(PointerPixels, 3);
                u32 PixelPtr4 = _mm256_extract_epi32(PointerPixels, 4);
                u32 PixelPtr5 = _mm256_extract_epi32(PointerPixels, 5);
                u32 PixelPtr6 = _mm256_extract_epi32(PointerPixels, 6);
                u32 PixelPtr7 = _mm256_extract_epi32(PointerPixels, 7);
                
                __m128i Texel0 = _mm_setr_epi32(((PixelPtr0 >> 24) & 0xFF),((PixelPtr0 >> 16) & 0xFF),
                                                ((PixelPtr0 >> 8) & 0xFF),((PixelPtr0 >> 0) & 0xFF));
                
                __m128i Texel1 = _mm_setr_epi32(((PixelPtr1 >> 24) & 0xFF), 
                                                ((PixelPtr1 >> 16) & 0xFF),((PixelPtr1 >> 8) & 0xFF),
                                                ((PixelPtr1 >> 0) & 0xFF));
                
                __m128i Texel2 = _mm_setr_epi32(((PixelPtr2 >> 24) & 0xFF),
                                                ((PixelPtr2 >> 16) & 0xFF),((PixelPtr2 >> 8) & 0xFF),
                                                ((PixelPtr2 >> 0) & 0xFF));
                
                __m128i Texel3 = _mm_setr_epi32(((PixelPtr3 >> 24) & 0xFF),
                                                ((PixelPtr3 >> 16) & 0xFF),((PixelPtr3 >> 8) & 0xFF),
                                                ((PixelPtr3 >> 0) & 0xFF));
                
                __m128i Texel4 = _mm_setr_epi32(((PixelPtr4 >> 24) & 0xFF),
                                                ((PixelPtr4 >> 16) & 0xFF),((PixelPtr4 >> 8) & 0xFF),
                                                ((PixelPtr4 >> 0) & 0xFF));
                
                __m128i Texel5 = _mm_setr_epi32(((PixelPtr5 >> 24) & 0xFF),
                                                ((PixelPtr5 >> 16) & 0xFF),((PixelPtr5 >> 8) & 0xFF),
                                                ((PixelPtr5 >> 0) & 0xFF));
                
                __m128i Texel6 = _mm_setr_epi32(((PixelPtr6 >> 24) & 0xFF),((PixelPtr6 >> 16) & 0xFF),((PixelPtr6 >> 8) & 0xFF),
                                                ((PixelPtr6 >> 0) & 0xFF));
                
                __m128i Texel7 = _mm_setr_epi32(((PixelPtr7 >> 24) & 0xFF),
                                                ((PixelPtr7 >> 16) & 0xFF),((PixelPtr7 >> 8) & 0xFF),
                                                ((PixelPtr7 >> 0) & 0xFF));
                
                
                __m256 TestPixeldX = _mm256_sub_ps(_mm256_cvtepi32_ps(PixelPX), StartX_x8);
                __m256 TestPixeldY = _mm256_sub_ps(_mm256_cvtepi32_ps(PixelPY), StartY_x8);
                
                __m256 Point = _mm256_add_ps(_mm256_mul_ps(TestPixeldX, NdX),_mm256_mul_ps(TestPixeldY, NdY));
                
                
                // PointLength < 0 //
                __m256i TotalMask = _mm256_set1_epi32(0);
                
                
                TotalMask = _mm256_castps_si256(_mm256_cmp_ps(Point, Zeroes, _CMP_LT_OQ));
                
                __m256 NewPixelDistance = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(TestPixeldX, TestPixeldX),
                                                                       _mm256_mul_ps(TestPixeldY,TestPixeldY)));
                
                PixelDistance = _mm256_or_ps(_mm256_and_ps(_mm256_castsi256_ps(TotalMask), NewPixelDistance),
                                             _mm256_andnot_ps(_mm256_castsi256_ps(TotalMask), PixelDistance));
                
                // PointLength > VLength //
                __m256i Mask;
                Mask = _mm256_castps_si256(_mm256_cmp_ps(Point, VLength_x8, _CMP_GT_OQ));
                
                TotalMask = _mm256_or_si256(TotalMask, Mask);
                
                __m256 TestLengthX = _mm256_sub_ps(_mm256_cvtepi32_ps(PixelPX), EndX_x8);
                __m256 TestLengthY = _mm256_sub_ps(_mm256_cvtepi32_ps(PixelPY), EndY_x8);
                NewPixelDistance = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(TestLengthX, TestLengthX), _mm256_mul_ps(TestLengthY,TestLengthY)));
                
                PixelDistance = _mm256_or_ps(_mm256_and_ps(_mm256_castsi256_ps(Mask), NewPixelDistance), _mm256_andnot_ps(_mm256_castsi256_ps(Mask), PixelDistance));
                
                NewPixelDistance = _mm256_add_ps(_mm256_mul_ps(TestPixeldX,PerpX),_mm256_mul_ps(TestPixeldY, PerpY));
                NewPixelDistance = _mm256_max_ps(NewPixelDistance, _mm256_mul_ps(NewPixelDistance, MinusOnes));
                
                TotalMask = _mm256_xor_si256(TotalMask, _mm256_cmpeq_epi32(_mm256_cvtps_epi32(Zeroes), _mm256_cvtps_epi32(Zeroes)));
                PixelDistance = _mm256_or_ps(_mm256_and_ps(_mm256_castsi256_ps(TotalMask), NewPixelDistance), _mm256_andnot_ps(_mm256_castsi256_ps(TotalMask), PixelDistance));
                PixelDistance = _mm256_sub_ps(PixelDistance, _mm256_sub_ps(LineWidth_x8, Ones));
                
                PixelDistance = _mm256_max_ps(PixelDistance, Zeroes);
                PixelDistance = _mm256_mul_ps(PixelDistance,PixelDistance);
                PixelDistance = _mm256_min_ps(PixelDistance, Ones);
                
                __m256i PixelDistancei = _mm256_cvtps_epi32(PixelDistance);
                __m256 OneMinusPixelDistances = _mm256_sub_ps(Ones, PixelDistance);
                __m256i OneMinusPixelDistancesi = _mm256_cvtps_epi32(_mm256_sub_ps(Ones, PixelDistance));
                __m256 t01, t23, t45, t67;
                __m256 pd01, pd23, pd45, pd67;
                __m256 Texel01, Texel23, Texel45, Texel67;
                Texel01 = Texel23 = Texel45 = Texel67 = {};
                pd01 = pd23 = pd45 = pd67 = {};
                t01 = t23 = t45 = t67 = {};
                
                Lo4(Texel01) = _mm_cvtepi32_ps(Texel0); 
                Hi4(Texel01) = _mm_cvtepi32_ps(Texel1); 
                Lo4(Texel23) = _mm_cvtepi32_ps(Texel2); 
                Hi4(Texel23) = _mm_cvtepi32_ps(Texel3); 
                Lo4(Texel45) = _mm_cvtepi32_ps(Texel4); 
                Hi4(Texel45) = _mm_cvtepi32_ps(Texel5); 
                Lo4(Texel67) = _mm_cvtepi32_ps(Texel6);
                Hi4(Texel67) = _mm_cvtepi32_ps(Texel7);
                
                Lo4(t01) = _mm_set1_ps(M(OneMinusPixelDistances, 0)); 
                Hi4(t01) = _mm_set1_ps(M(OneMinusPixelDistances, 1)); 
                Lo4(t23) = _mm_set1_ps(M(OneMinusPixelDistances, 2));
                Hi4(t23) = _mm_set1_ps(M(OneMinusPixelDistances, 3));
                Lo4(t45) = _mm_set1_ps(M(OneMinusPixelDistances, 4));
                Hi4(t45) = _mm_set1_ps(M(OneMinusPixelDistances, 5));
                Lo4(t67) = _mm_set1_ps(M(OneMinusPixelDistances, 6));
                Hi4(t67) = _mm_set1_ps(M(OneMinusPixelDistances, 7));
                
                
                Lo4(pd01) = _mm_set1_ps(M(PixelDistance, 0)); 
                Hi4(pd01) = _mm_set1_ps(M(PixelDistance, 1)); 
                Lo4(pd23) = _mm_set1_ps(M(PixelDistance, 2));
                Hi4(pd23) = _mm_set1_ps(M(PixelDistance, 3));
                Lo4(pd45) = _mm_set1_ps(M(PixelDistance, 4));
                Hi4(pd45) = _mm_set1_ps(M(PixelDistance, 5));
                Lo4(pd67) = _mm_set1_ps(M(PixelDistance, 6));
                Hi4(pd67) = _mm_set1_ps(M(PixelDistance, 7));
                
                Texel01 = _mm256_add_ps(_mm256_mul_ps(t01, _mm256_add_ps(_mm256_mul_ps(OneMinusT, Texel01), _mm256_mul_ps(t, Color_x8))), _mm256_mul_ps(pd01, Texel01));
                Texel23 = _mm256_add_ps(_mm256_mul_ps(t23, _mm256_add_ps(_mm256_mul_ps(OneMinusT, Texel23), _mm256_mul_ps(t, Color_x8))), _mm256_mul_ps(pd23, Texel23));
                Texel45 = _mm256_add_ps(_mm256_mul_ps(t45, _mm256_add_ps(_mm256_mul_ps(OneMinusT, Texel45), _mm256_mul_ps(t, Color_x8))), _mm256_mul_ps(pd45, Texel45));
                Texel67 = _mm256_add_ps(_mm256_mul_ps(t67, _mm256_add_ps(_mm256_mul_ps(OneMinusT, Texel67), _mm256_mul_ps(t, Color_x8))), _mm256_mul_ps(pd67, Texel67));
                
                __m256i a01 = _mm256_cvtps_epi32(Texel01);
                __m256i a23 = _mm256_cvtps_epi32(Texel23);
                __m256i a45 = _mm256_cvtps_epi32(Texel45);
                __m256i a67 = _mm256_cvtps_epi32(Texel67);
                
                __m256i shift0 = _mm256_slli_epi32(a01, 0);
                __m256i shift1 = _mm256_slli_epi32(a23, 8);
                __m256i shift2 = _mm256_slli_epi32(a45, 16);
                __m256i shift3 = _mm256_slli_epi32(a67, 24);
                
                shift0 = _mm256_or_si256(shift3, _mm256_or_si256(shift2, _mm256_or_si256(shift1, shift0)));
                
                
                
                __m256i shiftTest = _mm256_shuffle_epi8(shift0, _mm256_set_epi8(19,23,27,31,
                                                                                18,22,26,30,
                                                                                17,21,25,29,
                                                                                16,20,24,28,
                                                                                3,7,11,15,
                                                                                2,6,10,14,
                                                                                1,5,9,13,
                                                                                0,4,8,12));
                __m256i temp0 = {};
                __m256i temp1 = {};
                __m256i temp2 = {};
                
                temp0 = _mm256_insertf128_si256(temp0, _mm256_extractf128_si256(shiftTest, 1),0);
                
                temp1 = _mm256_unpacklo_epi32(shiftTest, temp0);
                temp2 = _mm256_unpackhi_epi32(shiftTest, temp0);
                
                _mm_maskstore_epi32(Ptr0, _mm256_extractf128_si256(WriteMask, 0),_mm256_castsi256_si128(temp1));
                _mm_maskstore_epi32(Ptr1, _mm256_extractf128_si256(WriteMask, 0),_mm256_castsi256_si128(temp2));
                
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

#if 0
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
ScanlineFill(game_buffer *Buffer, game_memory *Memory)
{
    v2 Start = V2(0,0);
    v2 End = V2(4,4);
    
    v2 Points[32] = 
    {
        V2(0,1)*Buffer->MetersToPixels,
        V2(2,0)*Buffer->MetersToPixels,
        V2(4,1)*Buffer->MetersToPixels,
    };
    u32 PointCount = 3;
    
    Start *= Buffer->MetersToPixels;
    End *= Buffer->MetersToPixels;
    
    if(1)
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
        
        
        for(s32 Y = (s32)MinY;
            Y < MaxY;
            ++Y)
        {
            v2 Scand = V2(0, 1);
            v2 ScanP = V2(0,Start.y + Y);
            
            f32 Hits[32] = {};
            u32 HitCount = 0;
            
            for(u32 PointIndex = 0;
                PointIndex < PointCount;
                ++PointIndex)
            {
                u32 NextPointIndex = (PointIndex + 1) % PointCount;
                v2 AB = Points[NextPointIndex] - Points[PointIndex];
                f32 Slope = Scand.x - (AB.x / (Scand.y - AB.y));
                f32 HitX = Points[PointIndex].x + (ScanP.y - Points[PointIndex].y) * Slope;
                if((Y >= Points[PointIndex].y) ^ (Y >= Points[NextPointIndex].y))
                {
                    Hits[HitCount++] = HitX;
                }
            }
            
            
            u32 HitIndex = 0;
            b32 IsInside = 0;
            
            for(s32 X = (s32)MinX;
                X < (s32)MaxX;
                ++X)
            {
                v2 PixelP = V2i(X, Y);
                v2 Pixeld = PixelP - Start;
                
                
                u32 *PixelPtrA = (u32 *)((u8 *)Pixels + 
                                         (u32)PixelP.x*sizeof(u32) +
                                         (u32)PixelP.y*Buffer->Pitch);
                
                if(HitIndex < HitCount)
                {
                    if(X >= Hits[HitIndex])
                    {
                        ++HitIndex;
                        IsInside = !IsInside;
                    }
                    
                }
                
                if(IsInside)
                    *PixelPtrA = 0xFFFFFFFF;
            }
            
        }
    }
}
#endif

#define ASTEROIDS_RENDER_H
#endif