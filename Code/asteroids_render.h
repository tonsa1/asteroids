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