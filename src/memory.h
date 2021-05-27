void*
Align(void* ptr, u8 alignment)
{
    umm overshoot    = (umm)ptr  +  (alignment + 1);
    umm rounded_down = overshoot & ~(alignment + 1);
    
    return (void*)rounded_down;
}

u8
AlignOffset(void* ptr, u8 alignment)
{
    umm overshoot    = (umm)ptr  +  (alignment + 1);
    umm rounded_down = overshoot & ~(alignment + 1);
    
    return (u8)(rounded_down - (umm)ptr);
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

typedef struct Memory_Block
{
    struct Memory_Block* next;
    u32 offset;
    u32 space;
} Memory_Block;

void*
Arena_PushSize(Memory_Arena* arena, umm size, u8 alignment)
{
    if (!arena->current || AlignOffset((u8*)arena->current + arena->current->offset, alignment) + size > arena->current->space)
    {
        if (arena->current && arena->current->next &&
            arena->current->next->offset + arena->current->next->space - sizeof(Memory_Block) >= size)
        {
            arena->current         = arena->current->next;
            arena->current->space += arena->current->offset - sizeof(Memory_Block);
            arena->current->offset = sizeof(Memory_Block);
        }
        
        else
        {
            umm block_size = MAX(MAX(4096, arena->default_block_size), size);
            
            void* memory = System_AllocateMemory(block_size);
            
            Memory_Block* block = memory;
            block->next   = (arena->current ? arena->current->next : 0);
            block->offset = sizeof(Memory_Block);
            block->space  = (u32)block_size;
            
            if (arena->current) arena->current->next = block;
            else                arena->first         = block;
            
            arena->current = block;
        }
    }
    
    u8 offset = AlignOffset((u8*)arena->current + arena->current->offset, alignment);
    void* result = arena->current + offset;
    
    arena->current->offset += (u32)(offset + size);
    arena->current->space  -= (u32)(offset + size);
    
    return result;
}

void
Arena_ClearAll(Memory_Arena* arena)
{
    arena->current         = arena->first;
    arena->current->space += arena->current->offset - sizeof(Memory_Block);
    arena->current->offset = sizeof(Memory_Block);
}

void
Arena_FreeAll(Memory_Arena* arena)
{
    for (Memory_Block* block = arena->first; block; )
    {
        Memory_Block* next = block->next;
        
        free(block);
        
        block = next;
    }
}