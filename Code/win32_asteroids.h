#if !defined(WIN32_ASTEROIDS_H)

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch; 
    int BytesPerPixel;
};

struct win32_window_dimension
{
    RECT ClientRect;
    int Width;
    int Height;
};


struct win32_game_code
{
    HMODULE GameCodeDLL;
    game_update_and_render *UpdateAndRender;
    FILETIME DLLLastWriteTime;
    
    b32 IsValid;
};



#define WIN32_ASTEROIDS_H
#endif
