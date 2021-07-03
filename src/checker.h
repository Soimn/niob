bool
TU_Init(TUnit* unit, Package_ID package, File_ID file, Statement* statement)
{
    bool encountered_errors = false;
    
    ZeroStruct(unit);
    unit->package   = package;
    unit->file      = file;
    unit->statement = statement;
    
    if      (statement->kind == Statement_ImportDecl)   unit->kind = TUnit_Import;
    else if (statement->kind == Statement_ForeignDecl)  unit->kind = TUnit_Foreign;
    else if (statement->kind == Statement_When)         unit->kind = TUnit_When;
    else if (statement->kind == Statement_VariableDecl) unit->kind = TUnit_Variable;
    else if (statement->kind == Statement_ConstantDecl)
    {
        Constant_Declaration* const_decl = (Constant_Declaration*)statement;
        
        if (const_decl->values->next == 0)
        {
            if      (const_decl->values->kind == Expr_Struct) unit->kind = TUnit_Struct;
            else if (const_decl->values->kind == Expr_Union)  unit->kind = TUnit_Union;
            else if (const_decl->values->kind == Expr_Enum)   unit->kind = TUnit_Enum;
            else if (const_decl->values->kind == Expr_Proc)
            {
                Proc_Expression* proc_expr = (Proc_Expression*)const_decl->values;
                
                if      (proc_expr->body != 0) unit->kind = TUnit_Proc;
                else if (proc_expr->is_decl)   unit->kind = TUnit_ProcDecl;
            }
        }
        
        if (unit->kind == TUnit_Invalid)
        {
            unit->kind = TUnit_Constant;
        }
    }
    
    else
    {
        //// ERROR: Illegal translation unit
        encountered_errors = true;
    }
    
    return !encountered_errors;
}

enum TU_CHECK_STATUS
{
    TUCheck_Error,
    TUCheck_Undecidable,
    TUCheck_Valid,
};

void
Checker_ReportError(Workspace* workspace, ...)
{
    NOT_IMPLEMENTED;
}

typedef struct Checker_State
{
    
} Checker_State;

bool
CheckExpression(Checker_State state, Expression* expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

bool
CheckStatement(Checker_State state, Statement* statement)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

Enum8(TU_CHECK_STATUS)
TU_Check(Workspace* workspace, TUnit* unit)
{
    Enum8(TU_CHECK_STATUS) result = TUCheck_Error;
    
    NOT_IMPLEMENTED;
    
    return result;
}