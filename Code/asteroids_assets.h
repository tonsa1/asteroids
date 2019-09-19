#if !defined(ASTEROIDS_ASSETS_H)

enum game_asset_id
{
    GAI_LineMeshes,
    GAI_LineMeshPlayer,
    GAI_LineMeshAsteroid,
    GAI_LineMeshBullet,
    GAI_SpriteRandom,
    GAI_CharacterA,
    
    GAI_Characters,
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
    
    GAI_Count,
};

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
    loaded_bitmap Bitmap;
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
    
    line_mesh LineMeshes[GAI_Count];
    
    u32 BitmapCount;
    asset_slot *Bitmaps;
    
    u32 AssetCount;
    asset *Assets;
    u32 TagCount;
    asset_tag *Tags;
    
    
    asset_slot Characters[26];
};

struct load_asset_work
{
    game_assets *Assets;
    //char* FileName;
    char FileName[64];
    game_asset_id ID;
    asset_slot *Slot;
    
    b32 HasAlignment;
    u32 AlignX;
    u32 TopDownAlignY;
    
    asset_state FinalState;
};

#define ASTEROIDS_ASSETS_H
#endif