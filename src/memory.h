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

#define MEMORY_ARENA_DEFAULT_BLOCK_SIZE (4096 - sizeof(Memory_Block))
typedef struct Memory_Block
{
    struct Memory_Block* next;
    u32 space;
    u32 offset;
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
            umm block_size = MAX(MAX(4096, arena->block_size), size + sizeof(Memory_Block));
            
            void* memory = System_AllocateMemory(block_size);
            
            Memory_Block* block = memory;
            block->next   = (arena->current ? arena->current->next : 0);
            block->offset = sizeof(Memory_Block);
            block->space  = (u32)(block_size - sizeof(Memory_Block));
            
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
        
        System_FreeMemory(block);
        
        block = next;
    }
}

typedef struct Bucket
{
    struct Bucket* prev;
    struct Bucket* next;
} Bucket;

void*
BA_Push(Bucket_Array* array)
{
    array->current_bucket_size += 1;
    
    if (array->current_bucket_size % array->bucket_capacity == 1)
    {
        if (array->current && array->current->next) array->current = array->current->next;
        else
        {
            Bucket* bucket = Arena_PushSize(array->arena, sizeof(Bucket) + array->bucket_capacity * array->element_size, ALIGNOF(Bucket));
            bucket->next = 0;
            bucket->prev = array->current;
            
            if (array->current) array->current->next = bucket;
            else                array->first         = bucket;
            array->current = bucket;
        }
        
        array->bucket_count        += 1;
        array->current_bucket_size %= array->bucket_capacity;
    }
    
    return (u8*)(array->current + 1) + (array->current_bucket_size - 1) * array->element_size;
}

void
BA_Pop(Bucket_Array* array, void* value)
{
    ASSERT(array->current_bucket_size > 0);
    
    array->current_bucket_size -= 1;
    if (value != 0) Copy((u8*)(array->current + 1) + array->current_bucket_size * array->element_size, value, array->element_size);
    
    if (array->current_bucket_size == 0 && array->current->prev != 0)
    {
        array->current             = array->current->prev;
        array->current_bucket_size = array->bucket_capacity;
        array->bucket_count       -= 1;
    }
}

umm
BA_ElementCount(Bucket_Array* array)
{
    return (array->bucket_count - 1) * array->bucket_capacity + array->current_bucket_size;
}

void*
BA_ElementAt(Bucket_Array* array, umm index)
{
    ASSERT(array->bucket_count > 0 && index < BA_ElementCount(array));
    
    umm bucket_index  = index / array->bucket_capacity;
    umm bucket_offset = index % array->bucket_capacity;
    
    
    Bucket* bucket = 0;
    if (bucket_index < BUCKET_ARRAY_BUCKET_CACHE_SIZE) bucket = array->bucket_cache[bucket_index];
    else
    {
        bucket = array->bucket_cache[BUCKET_ARRAY_BUCKET_CACHE_SIZE - 1]->next;
        for (umm i = BUCKET_ARRAY_BUCKET_CACHE_SIZE; i < bucket_index; ++i)
        {
            bucket = bucket->next;
        }
    }
    
    return (u8*)(bucket + 1) + bucket_offset * array->element_size;
}

void
BA_ClearAll(Bucket_Array* array)
{
    array->current             = array->first;
    array->current_bucket_size = 0;
    array->bucket_count        = 0;
}

typedef struct BA_Iterator
{
    umm index;
    Bucket* current_bucket;
    void* current;
} BA_Iterator;

BA_Iterator
BA_Iterate(Bucket_Array* array, BA_Iterator* prev_it)
{
    BA_Iterator it;
    
    if (prev_it == 0)
    {
        it = (BA_Iterator){
            .index = 0,
            .current_bucket = array->first,
            .current        = (array->current_bucket_size == 0 ? 0 : array->first + 1)
        };
    }
    
    else
    {
        it = *prev_it;
        it.index += 1;
        
        umm offset = it.index % array->bucket_capacity;
        if (offset)
        {
            it.current_bucket = it.current_bucket->next;
        }
        
        it.current = 0;
        if (it.current_bucket != array->current || offset < array->current_bucket_size)
        {
            it.current = (u8*)(it.current_bucket + 1) + offset * array->element_size;
        }
    }
    
    return it;
}