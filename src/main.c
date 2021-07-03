#include "niob.h" // API
#include "niob.c" // Implementation

int
main(int argc, const char** argv)
{
    Workspace_Options workspace_options = {
        0
    };
    
    Workspace* workspace = WS_Open(workspace_options);
    
    WS_AddFile(workspace, STR("main.n"));
    
    for (TUnit translation_unit; WS_RequestDeclaration(workspace, &translation_unit); )
    {
        // either this
        {
            // modify declaration
            
            WS_ResubmitDeclaration(workspace, translation_unit);
        }
        
        // or that
        {
            WS_CommitDeclaration(workspace, translation_unit);
        }
    }
    
    WS_GenerateCode(workspace);
    
    WS_Close(workspace);
}