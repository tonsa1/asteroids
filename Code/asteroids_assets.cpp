#define CHAR_A_ASCII_INDEX 65
#define GET_ASSET_INDEX_FROM_CODEPOINT(Codepoint) GAI_CharactersStart + 1 + (Codepoint - CHAR_A_ASCII_INDEX)
#define GET_CODEPOINT_FROM_ASSET_INDEX(BitmapIndex) GAI_CharactersStart + 1 + (BitmapIndex)

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

inline 
u32 AssetIndexFromGAI(game_asset_id ID, game_asset_id IDStart)
{
    u32 Result = ID - IDStart - 1;
    return Result;
}

internal
PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;
    
    switch(Work->ID)
    {
        
        case GAI_SpriteRandom:
        default:
        {
            if (Work->ID > GAI_BitmapStart && Work->ID < GAI_BitmapEnd)
            {
                loaded_bitmap Bitmap = DEBUGLoadBMP(DEBUGReadEntireFile, Work->Slot->FileName);
                if(Bitmap.Pixels)
                {
                    asset_slot *Slot = Work->Assets->LoadedAssets + Work->ID;
                    *Slot->Bitmap = Bitmap;
                    Slot->State = Work->FinalState;
                    PlatformPrintString("Thread loaded an asset", LogicalThreadIndex);
                }
            }
        } break;
    }
}

internal void
LoadAsset(game_assets *Assets, game_asset_id ID, asset_slot *Slot)
{
    if(AtomicCompareExchangeUInt32((u32 *)&Slot->State, AssetState_Unloaded, AssetState_Queued) ==
       AssetState_Unloaded)
    {
        load_asset_work *Work = PushStruct(&Assets->Arena, load_asset_work);
        Work->Assets = Assets;
        Work->ID = ID;
        Work->Slot = Slot;
        Work->FinalState = AssetState_Loaded;
        
        PlatformAddWorkQueueEntry(Assets->TranState->Queue, LoadAssetWork, Work);
    }
}

internal void
SetupLineMesh(game_assets *Assets, asset_slot *Slot, u32 LineCount)
{
    Slot->LineMesh = PushStruct(&Assets->Arena, line_mesh);
    line_mesh *LineMesh = Slot->LineMesh;
    LineMesh->LineCount = LineCount;
    LineMesh->Lines = PushArray(&Assets->Arena, LineCount, line);
}

inline line_mesh *GetLineMesh(game_assets *Assets, game_asset_id ID)
{
    asset_slot *Slot = Assets->LoadedAssets + ID;
    if(Slot->State == AssetState_Unloaded)
    {
        switch(ID)
        {
            case GAI_LineMeshBullet:
            {
                SetupLineMesh(Assets, Slot, 4);
                line_mesh *LineMesh = Slot->LineMesh;
                
                LineMesh->Lines[0] = CreateLine(V2(-0.5f, -0.0f), V2(-0.0f, 0.5f));
                LineMesh->Lines[1] = CreateLine(LineMesh->Lines[0].End, V2(0.5f, 0.0f));
                LineMesh->Lines[2] = CreateLine(LineMesh->Lines[1].End, V2(0.0f, -0.5f));;
                LineMesh->Lines[3] = CreateLine(LineMesh->Lines[2].End,LineMesh->Lines[0].Start);
                Slot->State = AssetState_Loaded;
            } break;
            
            case GAI_LineMeshBox:
            {
                SetupLineMesh(Assets, Slot, 8);
                line_mesh *LineMesh = Slot->LineMesh;
                
                
                LineMesh->Lines[0] = CreateLine(V2(-0.5f, 0.5f), V2(0.5f, 0.5f));
                LineMesh->Lines[1] = CreateLine(LineMesh->Lines[0].End, V2(0.5f, -0.5f));
                LineMesh->Lines[2] = CreateLine(LineMesh->Lines[1].End, V2(-0.5f, -0.5f));;
                LineMesh->Lines[3] = CreateLine(LineMesh->Lines[2].End,LineMesh->Lines[0].Start);
                
#if 1
                LineMesh->Lines[4] = LineMesh->Lines[0];
                LineMesh->Lines[4].Start *= 0.7f;
                LineMesh->Lines[4].End *= 0.7f;
                
                LineMesh->Lines[5] = LineMesh->Lines[1];
                LineMesh->Lines[5].Start *= 0.7f;
                LineMesh->Lines[5].End *= 0.7f;
                
                LineMesh->Lines[6] = LineMesh->Lines[2];
                LineMesh->Lines[6].Start *= 0.7f;
                LineMesh->Lines[6].End *= 0.7f;
                
                LineMesh->Lines[7] = LineMesh->Lines[3];
                LineMesh->Lines[7].Start *= 0.7f;
                LineMesh->Lines[7].End *= 0.7f;
#endif
                
                Slot->State = AssetState_Loaded;
            } break;
            
            default:
            {
                if (ID >= GAI_LineMeshAsteroidStart && ID <= GAI_LineMeshAsteroidEnd)
                {
                    SetupLineMesh(Assets, Slot, 5);
                    line_mesh *LineMesh = Slot->LineMesh;
                    
                    f32 Scale = 0.2f;
                    // TODO(Tony): Randomize asteroid line count
                    LineMesh->Lines[0] = CreateLine(V2(-0.5f, -0.5f) + RandomOffsetBilateral(Scale), V2(-0.4f, 0.4f) + RandomOffsetBilateral(Scale));
                    LineMesh->Lines[1] = CreateLine(LineMesh->Lines[0].End, V2(0.3f, 0.5f) + RandomOffsetBilateral(Scale));
                    LineMesh->Lines[2] = CreateLine(LineMesh->Lines[1].End, V2(0.6f, -0.1f) + RandomOffsetBilateral(Scale));;
                    LineMesh->Lines[3] = CreateLine(LineMesh->Lines[2].End, V2(0.1f, -0.6f) + RandomOffsetBilateral(Scale));;
                    LineMesh->Lines[4] = CreateLine(LineMesh->Lines[3].End,LineMesh->Lines[0].Start);
                    Slot->State = AssetState_Loaded;
                }
            } break;
        }
    }
    
    Assert(Slot->State == AssetState_Loaded);
    Assert(Slot->LineMesh);
    return Slot->LineMesh;
}

inline loaded_bitmap *GetBitmap(game_assets *Assets, game_asset_id ID)
{
    asset_slot *Slot = Assets->LoadedAssets + ID;
    if(Slot->State == AssetState_Unloaded)
    {
        LoadAsset(Assets, ID, Slot);
    }
    
    Assert(Slot->Bitmap);
    return Slot->Bitmap;
}

internal u32
PickBest(u32 InfoCount, asset_bitmap_info *Infos, asset_tag *Tags,
         f32 *MatchVector, f32 *WeightVector)
{
    f32 BestDiff = f32Max;
    u32 BestIndex = 0;
    for(u32 InfoIndex = 0;
        InfoIndex < InfoCount;
        ++InfoIndex)
    {
        asset_bitmap_info *Info = Infos + InfoIndex;
        
        f32 TotalWeightedDiff = 0.0f;
        for(u32 TagIndex = Info->FirstTagIndex;
            TagIndex < Info->OnePastLastTagIndex;
            ++TagIndex)
        {
            asset_tag *Tag = Tags + TagIndex;
            f32 Difference = MatchVector[Tag->ID] - Tag->Value;
            f32 Weighted = WeightVector[Tag->ID]*Absolute(Difference);
            TotalWeightedDiff += Weighted;
        }
        
        if(BestDiff > TotalWeightedDiff)
        {
            BestDiff = TotalWeightedDiff;
            BestIndex = InfoIndex;
        }
    }
    
    return BestIndex;
}

internal game_assets *
AllocateAssets(transient_state *TranState, memory_index Size)
{
    memory_arena *Arena = &TranState->TranArena;
    
    game_assets *Assets = PushStruct(Arena, game_assets);
    SubArena(&Assets->Arena, Arena, Size);
    Assets->TranState = TranState;
    
    u32 Test1 = GAI_LineMeshAsteroidStart;
    
    {
        //Assets->LineMeshes = PushArray(Arena, LINEMESH_COUNT, line_mesh);
        
        
        u32 LineCount = 3;
        Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh = PushStruct(Arena, line_mesh);
        Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh->LineCount = LineCount;
        Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh->Lines = PushArray(Arena, LineCount, line);
        Assets->LoadedAssets[GAI_LineMeshPlayer].State = AssetState_Loaded;
        
        line *Line1 = Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh->Lines + 0;
        line *Line2 = Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh->Lines + 1;
        line *Line3 = Assets->LoadedAssets[GAI_LineMeshPlayer].LineMesh->Lines + 2;
        Line1->Start = V2(-0.15f, -0.25f);
        Line1->End = V2(0, 0.25f);
        Line1->Color = V4(1.0f,1,1,1.0f);
        Line1->Vector = Line1->End - Line1->Start;
        Line1->Normal = Perp(Line1->Vector);
        Line1->Normal = Line1->Normal*(1 / Length(Line1->Normal));
        
        Line2->Start = V2(0.0f, 0.25f);
        Line2->End = V2(0.15f, -0.25f);
        Line2->Color = V4(1.0f,1,1,1.0f),
        Line2->Vector = Line2->End - Line2->Start;
        Line2->Normal = Perp(Line2->Vector);
        Line2->Normal = Line2->Normal*(1 / Length(Line2->Normal));
        
        Line3->Start = V2(0.15f, -0.25f);
        Line3->End = V2(-0.15f, -0.25f);
        Line3->Color = V4(1.0f,1,1,1.0f),
        Line3->Vector = Line3->End - Line3->Start;
        Line3->Normal = Perp(Line3->Vector);
        Line3->Normal = Line3->Normal*(1 / Length(Line3->Normal));
    }
    
    {
        u8 FileName[] = "chars/ .bmp";
        u32 FileNameLength = ArrayCount(FileName);
        for(u32 CodePoint = 'A';
            CodePoint <= 'Z';
            ++CodePoint)
        {
            asset_slot Slot = {};
            FileName[6] = CodePoint;
            //*Slot.Bitmap = DEBUGLoadBMP(DEBUGReadEntireFile, (char *)FileName);
            Slot.FileName = PushArray(&Assets->Arena, FileNameLength, char);
            CopySize(FileName, Slot.FileName, FileNameLength);
            
            u32 AssetIndex = GET_ASSET_INDEX_FROM_CODEPOINT(CodePoint);
            Assert(AssetIndex > GAI_CharactersStart && AssetIndex < GAI_CharactersEnd);
            Assets->LoadedAssets[AssetIndex] = Slot;
            Assets->LoadedAssets[AssetIndex].Bitmap = PushStruct(Arena, loaded_bitmap);
        }
    }
    
    Assets->TagCount = 0;
    Assets->Tags = 0;
    
    Assets->AssetCount = GAI_Count;
    
    return Assets;
}

