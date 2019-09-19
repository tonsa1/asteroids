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
    
    loaded_bitmap Bitmap = DEBUGLoadBMP(DEBUGReadEntireFile, Work->FileName);
    
    if(Bitmap.Pixels)
    {
        asset_slot *Slot = Work->Assets->Bitmaps + Work->ID;
        Slot->Bitmap = Bitmap;
        Slot->State = AssetState_Loaded;
        PlatformPrintString("Thread loaded an asset", LogicalThreadIndex);
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
        
        switch(ID)
        {
            case GAI_SpriteRandom:
            {
                CopySize("head.bmp", Work->FileName, 64);
            } break;
            
            default:
            {
                if(ID > GAI_Characters && ID <= GAI_Z)
                {
                    u8 FileName[12] = {'c','h','a','r','s','/',' ','.','b','m','p'};
                    FileName[6] = 65 + AssetIndexFromGAI(ID, GAI_Characters);
                    
                    CopySize(FileName, Work->FileName, 64);
                    //Work->FileName = FileName;
                }
            } break;
        }
        
        
        PlatformAddWorkQueueEntry(Assets->TranState->Queue, LoadAssetWork, Work);
    }
}

inline line_mesh *GetLineMesh(game_assets *Assets, game_asset_id ID)
{
    line_mesh *Result = Assets->LineMeshes + ID;
    
    return Result;
}

inline loaded_bitmap *GetBitmap(game_assets *Assets, game_asset_id ID)
{
    asset_slot *Slot = Assets->Bitmaps + ID;
    if(Slot->State == AssetState_Unloaded)
    {
        LoadAsset(Assets, ID, Slot);
    }
    
    return &Slot->Bitmap;
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
    
    
    
    line Line1 = {};
    line Line2 = {};
    line Line3 = {};
    //line Line4 = {};
    //line Line5 = {};
    //line Line6 = {};
    Assets->LineMeshes[GAI_LineMeshPlayer].LineCount = 3;
    Line1.Start = V2(-0.15f, -0.25f);
    Line1.End = V2(0, 0.25f);
    Line1.Color = V4(1.0f,1,1,1.0f);
    Line1.Vector = Line1.End - Line1.Start;
    Line1.Normal = Perp(Line1.Vector);
    Line1.Normal = Line1.Normal*(1 / Length(Line1.Normal));
    
    Line2.Start = V2(0.0f, 0.25f);
    Line2.End = V2(0.15f, -0.25f);
    Line2.Color = V4(1.0f,1,1,1.0f),
    Line2.Vector = Line2.End - Line2.Start;
    Line2.Normal = Perp(Line2.Vector);
    Line2.Normal = Line2.Normal*(1 / Length(Line2.Normal));
    
    Line3.Start = V2(0.15f, -0.25f);
    Line3.End = V2(-0.15f, -0.25f);
    Line3.Color = V4(1.0f,1,1,1.0f),
    Line3.Vector = Line3.End - Line3.Start;
    Line3.Normal = Perp(Line3.Vector);
    Line3.Normal = Line3.Normal*(1 / Length(Line3.Normal));
    
    Assets->LineMeshes[GAI_LineMeshPlayer].Lines[0] = Line1;
    Assets->LineMeshes[GAI_LineMeshPlayer].Lines[1] = Line2;
    Assets->LineMeshes[GAI_LineMeshPlayer].Lines[2] = Line3;
    
    Assets->BitmapCount = GAI_Count;
    Assets->Bitmaps = PushArray(Arena, Assets->BitmapCount, asset_slot);
    
#if 0
    u8 FileName[12] = {'c','h','a','r','s','/',' ','.','b','m','p'};
    for(u32 CodePoint = 'A';
        CodePoint < '[';
        ++CodePoint)
    {
        asset_slot Slot = {};
        Slot.State = AssetState_Loaded;
        FileName[6] = CodePoint;
        u8 Name[12] = "chars/A.bmp";
        Slot.Bitmap = DEBUGLoadBMP(DEBUGReadEntireFile, (char *)FileName);
        
        u32 BitmapIndex = GAI_Characters  + CodePoint - 65;
        Assets->Bitmaps[BitmapIndex] = Slot;
    }
#endif
    
    Assets->TagCount = 0;
    Assets->Tags = 0;
    
    Assets->AssetCount = Assets->BitmapCount;
    
    return Assets;
}

