#include "niob.h"

#define ASSERT(EX) ((EX) ? 1 : *(volatile int*)0)

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")
#define INVALID_DEFAULT_CASE ASSERT(!"INVALID_DEFAULT_CASE")
#define INVALID_CODE_PATH ASSERT(!"INVALID_CODE_PATH")

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define OFFSETOF(T, e) (umm)&((T*)0)->e
#define ALIGNOF(T) (u8)OFFSETOF(struct { char c; T t; }, t)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define STR(S) (String){.data = (u8*)(S), .size = sizeof(S) - 1}

#define U8_MAX  (u8) 0xFF
#define U16_MAX (u16)0xFFFF
#define U32_MAX (u32)0xFFFFFFFF
#define U64_MAX (u64)0xFFFFFFFFFFFFFFFF

#define I8_MAX  (i8) U8_MAX   >> 1
#define I16_MAX (i16)U16_MAX >> 1
#define I32_MAX (i32)U32_MAX >> 1
#define I64_MAX (i64)U64_MAX >> 1

#define I8_MIN  (i8) (1 << 7)
#define I16_MIN (i16)(1 << 15)
#define I32_MIN (i32)(1 << 31)
#define I64_MIN (i64)(1 << 63)

#include <wmmintrin.h>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#undef far
#undef near

void*
System_AllocateMemory(umm size)
{
    // TODO: handle running out of memory
    return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void
System_FreeMemory(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

bool
System_ReadFile(String path, Memory_Arena* arena, String* result)
{
    bool succeeded = false;
    
    NOT_IMPLEMENTED;
    
    return succeeded;
}

bool
System_FindNextFileInDir(String directory, void** state, String* file)
{
    bool found_file = false;
    
    NOT_IMPLEMENTED;
    
    return found_file;
}

#elif __linux__

void*
System_AllocateMemory(umm size)
{
    void* result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

void
System_FreeMemory(void* ptr)
{
    NOT_IMPLEMENTED;
}

#endif

#include "memory.h"
#include "string.h"

#include "lexer.h"
#include "checker.h"
#include "parser.h"

Workspace*
WS_Open(Workspace_Options options)
{
    static Workspace workspace_stub = {0};
    
    Workspace* workspace = &workspace_stub;
    
    void* memory = System_AllocateMemory(sizeof(Workspace));
    
    if (memory != 0)
    {
        workspace = memory;
        ZeroStruct(workspace);
        
        Path_Label predefined_labels[] = {
            { .name = STR("core"), .path = STR("./core") } // TODO: ./core
        };
        
        NOT_IMPLEMENTED;
    }
    
    return workspace;
}

void
WS_Close(Workspace* workspace)
{
    NOT_IMPLEMENTED;
}

void
WS_AddFile(Workspace* workspace, String file_path)
{
    String text;
    if (!System_ReadFile(file_path, &workspace->file_arena, &text))
    {
        //// ERROR: Failed to open file
    }
    
    else
    {
        umm element_count = BA_ElementCount(&workspace->files);
        ASSERT(element_count < ~(File_ID)0);
        
        File_ID file_id = (File_ID)element_count;
        File* file      = BA_Push(&workspace->files);
        
        file->path     = file_path;
        file->contents = text;
        
        *(File_ID*)BA_Push(&((Package*)BA_ElementAt(&workspace->packages, MAIN_PACKAGE))->files) = file_id;
        
        if (!ParseText(workspace, MAIN_PACKAGE, file_id, text, &workspace->arena, &workspace->tmp_arena, &workspace->arena))
        {
            //// ERROR
        }
    }
}

Package_ID
WS_AddPackage(Workspace* workspace, String package_path)
{
    // NOTE: the main package has no path, used as invalid value in this function
    Package_ID result = MAIN_PACKAGE;
    
    for (BA_Iterator it = BA_Iterate(&workspace->packages, 0);
         it.current != 0;
         it = BA_Iterate(&workspace->packages, &it))
    {
        Package* package = it.current;
        
        if (StringCompare(package_path, package->path))
        {
            result = (Package_ID)it.index;
            break;
        }
    }
    
    if (result == MAIN_PACKAGE)
    {
        // NOTE: Package does not already exist, import package
        
        umm element_count = BA_ElementCount(&workspace->packages);
        ASSERT(element_count < ~(Package_ID)0);
        
        result = (Package_ID)element_count;
        
        Package* package = BA_Push(&workspace->packages);
        ZeroStruct(package);
        
        package->name = DirectoryName(package_path);
        package->path = package_path;
        
        void* state = 0;
        String file_path;
        while (System_FindNextFileInDir(package_path, &state, &file_path))
        {
            String text;
            if (!System_ReadFile(file_path, &workspace->file_arena, &text))
            {
                //// ERROR: Failed to open file
                break;
            }
            
            else
            {
                element_count = BA_ElementCount(&workspace->files);
                ASSERT(element_count < ~(File_ID)0);
                
                File_ID file_id = (File_ID)element_count;
                File* file      = BA_Push(&workspace->files);
                
                file->path     = file_path;
                file->contents = text;
                
                *(File_ID*)BA_Push(&((Package*)BA_ElementAt(&workspace->packages, result))->files) = file_id;
                
                if (!ParseText(workspace, result, file_id, text, &workspace->arena, &workspace->tmp_arena, &workspace->arena))
                {
                    //// ERROR
                    break;
                }
            }
        }
    }
    
    return result;
}

bool
WS_RequestDeclaration(Workspace* workspace, TUnit* result)
{
    bool declarations_left = false;
    
    for (BA_Iterator it = BA_Iterate(&workspace->unchecked_units, 0);
         it.current != 0;
         it = BA_Iterate(&workspace->unchecked_units, &it))
    {
        TUnit* unit = it.current;
        
        if (unit->kind != TUnit_Invalid)
        {
            Enum8(TU_CHECK_STATUS) check_status = TU_Check(workspace, unit);
            
            if (check_status == TUCheck_Valid)
            {
                *result = *unit;
                
                BA_Remove(&workspace->unchecked_units, it.index);
                
                // NOTE: There might not be any declarations left once this is removed, but
                //       assume there is and check on the next call to this function
                declarations_left = true;
                
                break;
            }
            
            else if (check_status == TUCheck_Error) break;
            else                                    continue;
        }
    }
    
    return declarations_left;
}

void
WS_ResubmitDeclaration(Workspace* workspace, TUnit old_unit)
{
    TUnit* unit = BA_Add(&workspace->unchecked_units);
    
    if (!TU_Init(unit, old_unit.package, old_unit.file, old_unit.statement))
    {
        //// ERROR:
    }
}

void
WS_CommitDeclaration(Workspace* workspace, TUnit old_unit)
{
    TUnit* unit = BA_Add(&workspace->committed_units);
    
    if (!TU_Init(unit, old_unit.package, old_unit.file, old_unit.statement))
    {
        //// ERROR:
    }
    
    else
    {
        unit->symbol_table = old_unit.symbol_table;
        NOT_IMPLEMENTED;
    }
}

void
WS_GenerateCode(Workspace* workspace)
{
    NOT_IMPLEMENTED;
}

Identifier
WS_GetIdentifier(Workspace* workspace, String string)
{
    Identifier result = 0;
    
    // TODO: BLANK_IDENTIFIER must refer to _
    NOT_IMPLEMENTED;
    
    return result;
}