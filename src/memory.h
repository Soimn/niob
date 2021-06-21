void*
Align(void* ptr, u8 alignment)
{
    umm overshoot    = (umm)ptr  +  (alignment - 1);
    umm rounded_down = overshoot & ~(alignment - 1);
    
    return (void*)rounded_down;
}

u8
AlignOffset(void* ptr, u8 alignment)
{
    umm overshoot    = (umm)ptr  +  (alignment - 1);
    umm rounded_down = overshoot & ~(alignment - 1);
    
    return (u8)(rounded_down - (umm)ptr);
}

umm
RoundSize(umm size, u8 alignment)
{
    umm overshoot    = size      +  (alignment - 1);
    umm rounded_down = overshoot & ~(alignment - 1);
    
    return rounded_down;
}

void
Copy(void* src, void* dest, umm size)
{
    for (umm i = 0; i < size; ++i)
    {
        ((u8*)dest)[i] = ((u8*)src)[i];
    }
}

void
CopyOverlapped(void* src, void* dest, umm size)
{
    if ((umm)src > (umm)dest) Copy(src, dest, size);
    else
    {
        for (umm i = 1; i < size + 1; ++i)
        {
            ((u8*)dest)[size - i] = ((u8*)src)[size - i];
        }
    }
}

void
Zero(void* ptr, umm size)
{
    for (umm i = 0; i < size; ++i) *(u8*)ptr = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(*(S)))