#if !defined(ASTEROIDS_ASSETS_H)

enum game_asset_id
{
    GAI_LineMeshStart,
    GAI_LineMeshAsteroidStart = 0,
    GAI_LineMeshAsteroid,
    GAI_LineMeshAsteroidEnd = 20,
    GAI_LineMeshPlayer,
    GAI_LineMeshBullet,
    GAI_LineMeshBox,
    GAI_LineMeshEnd,
    
    GAI_BitmapStart,
    GAI_SpriteRandom,
    GAI_CharactersStart,
    GAI_A,
    GAI_B,
    GAI_C,
    GAI_D,
    GAI_E,
    GAI_F,
    GAI_G,
    GAI_H,
    GAI_I,
    GAI_J,
    GAI_K,
    GAI_L,
    GAI_M,
    GAI_N,
    GAI_O,
    GAI_P,
    GAI_Q,
    GAI_R,
    GAI_S,
    GAI_T,
    GAI_U,
    GAI_V,
    GAI_W,
    GAI_X,
    GAI_Y,
    GAI_Z,
    GAI_CharactersEnd,
    GAI_BitmapEnd,
    
    GAI_Count,
};

#define BITMAP_COUNT (GAI_BitmapEnd - GAI_BitmapStart)
#define LINEMESH_COUNT (GAI_LineMeshEnd - GAI_LineMeshStart)
#define MAX_ASTEROID_MESH_COUNT (GAI_LineMeshAsteroidEnd - GAI_LineMeshAsteroidStart)

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued, 
    AssetState_Loaded,
    AssetState_Locked,
};


struct asset_slot
{
    asset_state State;
    char *FileName;
    
    union
    {
        loaded_bitmap *Bitmap;
        line_mesh *LineMesh;
    };
};

enum asset_tag_id
{
    Tag_Random,
};

struct asset_tag
{
    game_asset_id ID;
    u32 Value;
};

struct asset_type
{
    u32 Count;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct asset_bitmap_info
{
    v2 AlignPercentage;
    f32 WidthOverWidth;
    u32 Width;
    u32 Height;
    
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
};

struct asset_group
{
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
};

struct asset
{
    u32 FirstTagIndex;
    u32 OnePAstLastTagIndex;
    u32 SlotID;
};

struct game_assets
{
    memory_arena Arena;
    struct transient_state *TranState;
    
    u32 RandomAsteroidCount;
    line_mesh *RandomAsteroidMeshes;
    
    //loaded_bitmap *Bitmaps;
    //asset_slot Characters[26];
    
    u32 AssetCount;
    asset *AssetList;
    u32 TagCount;
    asset_tag *Tags;
    
    asset_slot LoadedAssets[GAI_Count];
};

struct load_asset_work
{
    game_assets *Assets;
    game_asset_id ID;
    asset_slot *Slot;
    
    b32 HasAlignment;
    u32 AlignX;
    u32 TopDownAlignY;
    
    asset_state FinalState;
};

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

#define ASTEROIDS_ASSETS_H
#endif