#if !defined(ASTEROIDS_INTRINSICS_H)

inline u32 AtomicCompareExchangeUInt32(u32 volatile *Value, u32 Expected, u32 New)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, Expected, New);
    
    return Result;
}

inline
u32 GetStringLength(void *Pointer)
{
    u32 Result = 0;
    for(u8 *Byte = (u8 *)Pointer;
        *Byte;
        ++Byte)
    {
        ++Result;
    }
    
    return Result;
}

inline void 
CopySize(void *Source, void *Dest_, u32 Size)
{
    u8 *Dest = (u8 *)Dest_;
    u32 Index = 0;
    for(u8 *Byte = (u8 *)Source;
        Byte && (*Byte && Index < Size);
        ++Byte)
    {
        *(Dest + Index++) = *Byte;
    }
}

inline void 
ZeroSize(void *Pointer, memory_index Size)
{
    memory_index Index = 0;
    for(u8 *Byte = (u8 *)Pointer;
        Index < Size;
        ++Byte)
    {
        *(Byte + Index++) = 0;
    }
}


#define ASTEROIDS_INTRINSICS_H
#endif