#include "niob.h" // API
#include "niob.c" // Implementation

int
main(int argc, const char** argv)
{
    Workspace* workspace = WS_Open();
    
    WS_AddFile(workspace, STR("main.n"));
    
    for (Declaration declaration; WS_RequestDeclaration(workspace, &declaration); )
    {
        // either this
        {
            // modify declaration
            
            WS_ResubmitDeclaration(workspace, declaration);
        }
        
        // or that
        {
            WS_CommitDeclaration(workspace, declaration);
        }
    }
    
    WS_GenerateCode(workspace);
    
    WS_Close(workspace);
}