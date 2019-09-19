#include <stdlib.h>

#pragma pack(push,1)
struct bitmap_header
{
    u16 FileType;
    u16 Reserved1;
    u16 Reserved2;
	u32 FileSize;
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

struct image_u32
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

internal void
WriteImage(image_u32 Image, char* Filename)
{
    u32 OutputPixelSize = sizeof(u32)*Image.Width*Image.Height;
    bitmap_header Header = {};
    Header.FileType = 0x4D42;
    Header.FileSize = sizeof(Header) + OutputPixelSize;
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.Width = Image.Width;
    Header.Height = Image.Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = OutputPixelSize;
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorsUsed = 0;
    Header.ColorsImportant = 0;
    Header.RedMask = 0x000000FF;
    Header.GreenMask = 0x0000FF00;
    Header.BlueMask = 0x00FF0000;
    FILE *OutFile = fopen(Filename, "wb");
    if(OutFile)
    {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Image.Pixels, OutputPixelSize, 1, OutFile);
        fclose(OutFile);
    }
    else
    {
        fprintf(stderr, "[ERROR] Unable to write output file %s.\n", Filename);
    }
    
}



debug_read_file_result result = DEBUGPlatformReadEntireFile("Abuela.ttf");
loaded_bitmap Result = {};
u8 FileName[5] = {' ','.','b','m','p'};
for(u32 CodePoint = 'A';
    CodePoint < 'B'; //'[';
    ++CodePoint)
{
    wchar_t CPoint = (wchar_t)CodePoint;
    
    HDC DeviceContext = 0;
    
    if(!DeviceContext)
    {
        DeviceContext = CreateCompatibleDC(0);
        HBITMAP DeviceBitmap = CreateCompatibleBitmap(DeviceContext, 1024, 1024);
        SelectObject(DeviceContext, DeviceBitmap);
        SetBkColor(DeviceContext, RGB(0, 0, 0));
        
        TEXTMETRIC TextMetric;
        GetTextMetrics(DeviceContext, &TextMetric);
    }
    
    SIZE Size;
    GetTextExtentPoint32W(DeviceContext, &CPoint, 1, &Size);
    
    u32 Width = Size.cx;
    u32 Height = Size.cy;
    
    SetTextColor(DeviceContext, RGB(255, 255, 255));
    TextOutW(DeviceContext, 0, 0, &CPoint, 1);
    
    
    Result.Width = Width;
    Result.Height = Height;
    Result.Pitch = Result.Width*4;
    Result.Pixels = (u8 *)malloc(Result.Height * Result.Pitch);
    
    u8 *DestRow = (u8 *)Result.Pixels + ((Result.Height - 1) * Result.Pitch);
    for(s32 Y = 0;
        Y < Result.Height;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        for(s32 X = 0;
            X< Result.Width;
            ++X)
        {
            COLORREF Pixel = GetPixel(DeviceContext, X, Y);
            u8 Alpha = (u8)(Pixel & 0xFF);
            
            *Dest++ = ((Alpha << 24) | 
                       (Alpha << 16) | 
                       (Alpha << 8) | 
                       (Alpha << 0));
            
        }
        
        DestRow -= Result.Pitch;
    }
    
#if 1
    
    FileName[0] = CodePoint;
    
    image_u32 Image = {};
    Image.Width = Result.Width;
    Image.Height = Result.Height;
    Image.Pixels = (u32 *)Result.Pixels;
    WriteImage(Image, (char *)FileName);
#endif 
}