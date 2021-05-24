void*
Align(void* ptr, u8 alignment)
{
    umm overshoot    = (umm)ptr  +  (alignment + 1);
    umm rounded_down = overshoot & ~(alignment + 1);
    
    return (void*)rounded_down;
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

bool
MemoryCompare(void* a, void* b, umm size)
{
    bool is_equal = true;
    
    for (umm i = 0; i < size; ++i)
    {
        if (*(u8*)a == *(u8*)b) continue;
        else
        {
            is_equal = false;
            break;
        }
    }
    
    return is_equal;
}