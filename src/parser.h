typedef struct Parser_State
{
    Memory_Arena* arena;
    Bucket_Array(Token)* tokens;
    BA_Iterator current_token;
} Parser_State;

Token
GetToken(Parser_State state)
{
    return *(Token*)state.current_token.current;
}

Token
PeekToken(Parser_State state)
{
    Token* token = state.current_token.current;
    
    BA_Iterator peek_it = BA_Iterate(state.tokens, &state.current_token);
    if (peek_it.current)
    {
        token = peek_it.current;
    }
    
    return *token;
}

Token
PeekNextToken(Parser_State state)
{
    Token* token = state.current_token.current;
    
    BA_Iterator peek_it = BA_Iterate(state.tokens, &state.current_token);
    if (peek_it.current)
    {
        token = peek_it.current;
        
        peek_it = BA_Iterate(state.tokens, &peek_it);
        if (peek_it.current)
        {
            token = peek_it.current;
        }
    }
    
    return *token;
}

void
SkipPastCurrentToken(Parser_State state)
{
    if (state.current_token.current)
    {
        state.current_token = BA_Iterate(state.tokens, &state.current_token);
    }
}

umm
Sizeof_Expression(Enum8(EXPRESSION_KIND) kind)
{
    ASSERT(kind != Expr_Invalid);
    
    umm size       = 0;
    umm precedence = kind / 20;
    
    if (precedence == 3 || precedence == 1 && kind != Expr_ArrayType || kind == Expr_PostIncrement || kind == Expr_PostDecrement)
    {
        size = sizeof(Unary_Expression);
    }
    else if (precedence - 4 <= 5 && kind != Expr_InfixCall) size = sizeof(Binary_Expression);
    else if (kind == Expr_ArrayType)                        size = sizeof(ArrayType_Expression);
    else if (kind == Expr_Call || kind == Expr_InfixCall)   size = sizeof(Call_Expression);
    else if (kind == Expr_Subscript)                        size = sizeof(Subscript_Expression);
    else if (kind == Expr_Slice)                            size = sizeof(Slice_Expression);
    else if (kind == Expr_ElementOf)                        size = sizeof(ElementOf_Expression);
    else if (kind == Expr_StructLiteral)                    size = sizeof(StructLiteral_Expression);
    else if (kind == Expr_ArrayLiteral)                     size = sizeof(ArrayLiteral_Expression);
    else if (kind == Expr_Proc)                             size = sizeof(Proc_Expression);
    else if (kind == Expr_Struct)                           size = sizeof(Struct_Expression);
    else if (kind == Expr_Enum)                             size = sizeof(Enum_Expression);
    else                                                    size = sizeof(BasicLiteral_Expression);
    
    return size;
}

void*
PushExpression(Parser_State state, Enum8(EXPRESSION_KIND) kind)
{
    umm size = Sizeof_Expression(kind);
    
    Expression* expression = Arena_PushSize(state.arena, size, ALIGNOF(Expression));
    
    Zero(expression, size);
    expression->kind = kind;
    
    return expression;
}

umm
Sizeof_Statement(Enum8(STATEMENT_KIND) kind)
{
    ASSERT(kind != Statement_Invalid);
    
    umm size = 0;
    
    if      (kind == Statement_Expression)                          size = sizeof(Expression_Statement);
    else if (kind == Statement_Block)                               size = sizeof(Block_Statement);
    else if (kind == Statement_If || kind == Statement_When)        size = sizeof(If_Statement);
    else if (kind == Statement_While)                               size = sizeof(While_Statement);
    else if (kind == Statement_For)                                 size = sizeof(For_Statement);
    else if (kind == Statement_Break || kind == Statement_Continue) size = sizeof(ScopeControl_Statement);
    else if (kind == Statement_Defer)                               size = sizeof(Defer_Statement);
    else if (kind == Statement_Using)                               size = sizeof(Using_Statement);
    else if (kind == Statement_Return)                              size = sizeof(Return_Statement);
    else if (kind == Statement_Assignment)                          size = sizeof(Assignment_Statement);
    else if (kind == Statement_VariableDecl)                        size = sizeof(Variable_Declaration);
    else if (kind == Statement_ConstantDecl)                        size = sizeof(Constant_Declaration);
    else                                                            size = sizeof(Import_Declaration);
    
    return size;
}

void*
PushStatement(Parser_State state, Enum8(STATEMENT_KIND) kind, Any_Statement* memory)
{
    umm size = Sizeof_Statement(kind);
    
    Statement* statement = 0;
    
    if (memory) statement = (Statement*)memory;
    else        statement = Arena_PushSize(state.arena, size, ALIGNOF(Statement));
    
    Zero(statement, size);
    statement->kind = kind;
    
    return statement;
}

bool ParseExpression(Parser_State state, Expression** expression);
bool ParseBlockStatement(Parser_State state, Block_Statement* block);

bool
ParseArgumentList(Parser_State state, Argument** arguments)
{
    bool encountered_errors = false;
    
    Argument* prev_arg = 0;
    while (!encountered_errors)
    {
        Argument* argument = Arena_PushSize(state.arena, sizeof(Argument), ALIGNOF(Argument));
        ZeroStruct(argument);
        
        if (prev_arg == 0) *arguments     = argument;
        else               prev_arg->next = argument;
        prev_arg = argument;
        
        if (!ParseExpression(state, &argument->value)) encountered_errors = true;
        else
        {
            Token token = GetToken(state);
            
            if (token.kind == Token_Equals)
            {
                SkipPastCurrentToken(state);
                
                argument->name = argument->value;
                
                if (!ParseExpression(state, &argument->value))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                token = GetToken(state);
                if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                else break;
            }
        }
        
    }
    
    return !encountered_errors;
}

bool
ParseAttributes(Parser_State state, Attribute** attributes)
{
    bool encountered_errors = false;
    
    Attribute* prev_attribute = 0;
    while (!encountered_errors)
    {
        Token token = GetToken(state);
        
        if (token.kind != Token_At) break;
        else
        {
            Attribute* attribute = Arena_PushSize(state.arena, sizeof(Attribute), ALIGNOF(Attribute));
            ZeroStruct(attribute);
            
            if (prev_attribute == 0) attributes           = attribute;
            else                     prev_attribute->next = attribute;
            prev_attribute = attribute;
            
            SkipPastCurrentToken(state);
            token = GetToken(state);
            
            if (token.kind != Token_Identifier)
            {
                //// ERROR: Missing name of attribute
                encountered_errors = true;
            }
            
            else if (token.identifier == BLANK_IDENTIFIER)
            {
                //// ERROR: Invalid use of blank identifier as attribute name
                encountered_errors = true;
            }
            
            else
            {
                attribute->name = token.identifier;
                
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                if (token.kind == Token_OpenParen)
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind != Token_CloseParen)
                    {
                        if (!ParseArgumentList(state, &attribute->arguments))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing closing parenthesis
                            encountered_errors = true;
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseParameterList(Parser_State state, Parameter** parameters, bool* has_param_names)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_OpenParen)
    {
        SkipPastCurrentToken(state);
        token = GetToken(state);
        
        if (token.kind != Token_CloseParen)
        {
            Parameter* prev_param = 0;
            while (!encountered_errors)
            {
                token = GetToken(state);
                
                bool is_using = false;
                if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
                {
                    SkipPastCurrentToken(state);
                    
                    is_using = true;
                }
                
                Expression* names     = 0;
                Expression* prev_name = 0;
                while (!encountered_errors)
                {
                    Expression* name;
                    if (!ParseExpression(state, &name)) encountered_errors = true;
                    else
                    {
                        if (prev_name == 0) names            = name;
                        else                prev_name->next  = name;
                        prev_name = name;
                        
                        token = GetToken(state);
                        if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                        else break;
                    }
                }
                
                token = GetToken(state);
                if (token.kind == Token_Equals || token.kind != Token_Colon && prev_param != 0)
                {
                    //// ERROR: Missing type of parameter
                    encountered_errors = true;
                }
                
                else if (token.kind != Token_Colon)
                {
                    if (is_using)
                    {
                        //// ERROR: Using is not applicable to parameters in a procedure type
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        // NOTE: procedure wihout parameter names
                        *has_param_names = false;
                        
                        Expression* type = names;
                        while (type != 0)
                        {
                            Expression* next_type = type->next;
                            type->next = 0;
                            
                            Parameter* parameter = Arena_PushSize(state.arena, sizeof(Parameter), ALIGNOF(Parameter));
                            ZeroStruct(parameter);
                            
                            if (prev_param == 0) *parameters      = parameter;
                            else                 prev_param->next = parameter;
                            prev_param = parameter;
                            
                            parameter->type = type;
                            
                            type = next_type;
                        }
                    }
                }
                
                else
                {
                    Parameter* parameter = Arena_PushSize(state.arena, sizeof(Parameter), ALIGNOF(Parameter));
                    ZeroStruct(parameter);
                    
                    if (prev_param == 0) *parameters      = parameter;
                    else                 prev_param->next = parameter;
                    prev_param = parameter;
                    
                    *has_param_names = true;
                    
                    parameter->names    = names;
                    parameter->is_using = is_using;
                    
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind != Token_Equals)
                    {
                        if (!ParseExpression(state, &parameter->type))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        if (token.kind == Token_Equals)
                        {
                            SkipPastCurrentToken(state);
                            
                            if (!ParseExpression(state, &parameter->value))
                            {
                                encountered_errors = true;
                            }
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(state);
                    if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                    else break;
                }
            }
        }
        
        if (!encountered_errors)
        {
            token = GetToken(state);
            
            if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
            else
            {
                //// ERROR: Missing closing parenthesis after parameter list
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParsePrimaryExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_String)
    {
        *expression = PushExpression(state, Expr_String);
        ((BasicLiteral_Expression*)*expression)->string = token.string;
        
        SkipPastCurrentToken(state);
    }
    
    else if (token.kind == Token_Character)
    {
        *expression = PushExpression(state, Expr_Char);
        ((BasicLiteral_Expression*)*expression)->character = token.character;
        
        SkipPastCurrentToken(state);
    }
    
    else if (token.kind == Token_Int)
    {
        *expression = PushExpression(state, Expr_Int);
        ((BasicLiteral_Expression*)*expression)->integer = token.integer;
        
        SkipPastCurrentToken(state);
    }
    
    else if (token.kind == Token_Float)
    {
        *expression = PushExpression(state, Expr_Float);
        ((BasicLiteral_Expression*)*expression)->floating = token.floating;
        
        SkipPastCurrentToken(state);
    }
    
    else if (token.kind == Token_Underscore)
    {
        *expression = PushExpression(state, Expr_Identifier);
        ((BasicLiteral_Expression*)*expression)->identifier = BLANK_IDENTIFIER;
        
        SkipPastCurrentToken(state);
    }
    
    else if (token.kind == Token_Identifier)
    {
        if (token.keyword == Keyword_Invalid)
        {
            *expression = PushExpression(state, Expr_Identifier);
            ((BasicLiteral_Expression*)*expression)->identifier = token.identifier;
            
            SkipPastCurrentToken(state);
        }
        
        else if (token.keyword == Keyword_True || token.keyword == Keyword_False)
        {
            *expression = PushExpression(state, Expr_Boolean);
            ((BasicLiteral_Expression*)*expression)->boolean = (token.keyword == Keyword_True);
            
            SkipPastCurrentToken(state);
        }
        
        else if (token.keyword == Keyword_Proc)
        {
            Proc_Expression* proc_expr = PushExpression(state, Expr_Proc);
            *expression = (Expression*)proc_expr;
            
            SkipPastCurrentToken(state);
            token = GetToken(state);
            
            
            bool has_param_names = false;
            if (token.kind == Token_OpenParen)
            {
                if (!ParseParameterList(state, &proc_expr->parameters, &has_param_names))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                token = GetToken(state);
                if (token.kind == Token_Arrow)
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind == Token_OpenParen)
                    {
                        SkipPastCurrentToken(state);
                        
                        Return_Value* prev_value = 0;
                        while (!encountered_errors)
                        {
                            token = GetToken(state);
                            if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
                            {
                                //// ERROR: Using is not applicable on return values
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                Return_Value* return_value = Arena_PushSize(state.arena, sizeof(Return_Value), ALIGNOF(Return_Value));
                                ZeroStruct(return_value);
                                
                                if (prev_value == 0) proc_expr->return_values = return_value;
                                else                 prev_value->next         = return_value;
                                prev_value = return_value;
                                
                                if (!ParseExpression(state, &return_value->type)) encountered_errors = true;
                                else
                                {
                                    token = GetToken(state);
                                    if (token.kind == Token_Colon)
                                    {
                                        SkipPastCurrentToken(state);
                                        token = GetToken(state);
                                        
                                        if (token.kind == Token_Equals)
                                        {
                                            //// ERROR: Return values cannot be assigned default values
                                            encountered_errors = true;
                                        }
                                        
                                        else
                                        {
                                            return_value->name = return_value->type;
                                            
                                            if (!ParseExpression(state, &return_value->type)) encountered_errors = true;
                                            else
                                            {
                                                if (token.kind != Token_Equals) break;
                                                else
                                                {
                                                    //// ERROR: Return values cannot be assigned default values
                                                    encountered_errors = true;
                                                }
                                            }
                                        }
                                    }
                                    
                                    token = GetToken(state);
                                    if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                                    else break;
                                }
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            token = GetToken(state);
                            if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
                            else
                            {
                                //// ERROR: Missing matching closing parenthesis
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    else
                    {
                        Return_Value* return_value = Arena_PushSize(state.arena, sizeof(Return_Value), ALIGNOF(Return_Value));
                        ZeroStruct(return_value);
                        
                        proc_expr->return_values = return_value;
                        
                        token = GetToken(state);
                        if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
                        {
                            //// ERROR: Using is not applicable on return values
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            if (!ParseExpression(state, &return_value->type)) encountered_errors = true;
                            else
                            {
                                token = GetToken(state);
                                if (token.kind == Token_Equals)
                                {
                                    //// ERROR: Return values cannot be assigned default values
                                    encountered_errors = true;
                                }
                                
                                else if (token.kind == Token_Comma)
                                {
                                    //// ERROR: Multiple return values must be enclosed in parenthesis
                                    encountered_errors = true;
                                }
                            }
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(state);
                    if (token.kind == Token_Identifier && token.keyword == Keyword_Where)
                    {
                        SkipPastCurrentToken(state);
                        
                        if (!ParseExpression(state, &proc_expr->polymorph_condition))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_TripleMinus)
                        {
                            SkipPastCurrentToken(state);
                            
                            proc_expr->is_decl = true;
                        }
                        
                        else if (token.kind == Token_OpenBrace || token.kind == Token_Identifier && token.keyword == Keyword_Do)
                        {
                            if (!has_param_names)
                            {
                                //// ERROR: Only procedure types and declarations may omit parameter names
                                encountered_errors = true;
                            }
                            
                            else if (!ParseBlockStatement(state, proc_expr->body))
                            {
                                encountered_errors = true;
                            }
                        }
                        
                        // NOTE: Expr_Proc and Expr_ProcType share the same structure
                        else
                        {
                            proc_expr->kind = Expr_ProcType;
                            
                            if (proc_expr->polymorph_condition != 0)
                            {
                                //// ERROR: Where clause polymorphic conditions are not allowed on procedure types
                                encountered_errors = true;
                            }
                        }
                    }
                }
            }
        }
        
        else if (token.keyword == Keyword_Struct || token.keyword == Keyword_Union)
        {
            Struct_Expression* struct_expr = PushExpression(state, token.keyword == Keyword_Struct ? Expr_Struct : Expr_Union);
            *expression = (Expression*)struct_expr;
            
            SkipPastCurrentToken(state);
            token = GetToken(state);
            
            if (token.kind == Token_OpenParen)
            {
                bool has_param_names = false; // TODO: Should this be used?
                if (!ParseParameterList(state, &struct_expr->parameters, &has_param_names)) encountered_errors = true;
                else
                {
                    token = GetToken(state);
                    
                    if (token.kind == Token_Identifier && token.keyword == Keyword_Where)
                    {
                        SkipPastCurrentToken(state);
                        
                        if (!ParseExpression(state, &struct_expr->polymorph_condition))
                        {
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            else if (token.kind == Token_Identifier && token.keyword == Keyword_Where)
            {
                //// ERROR: Polymorph condition without parameter list
                encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                token = GetToken(state);
                
                if (token.kind == Token_TripleMinus)
                {
                    SkipPastCurrentToken(state);
                    
                    struct_expr->is_decl = true;
                }
                
                else if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
                {
                    //// ERROR: Do is not applicable on structs
                    encountered_errors = true;
                }
                
                else if (token.kind != Token_OpenBrace)
                {
                    //// ERROR: Missing struct body
                    encountered_errors = true;
                }
                
                else
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind != Token_CloseBrace)
                    {
                        Struct_Member* prev_member = 0;
                        while (!encountered_errors)
                        {
                            Struct_Member* struct_member = Arena_PushSize(state.arena, sizeof(Struct_Member), ALIGNOF(Struct_Member));
                            ZeroStruct(struct_member);
                            
                            if (prev_member == 0) struct_expr->members = struct_member;
                            else                  prev_member->next    = struct_member;
                            prev_member = struct_member;
                            
                            token = GetToken(state);
                            
                            if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
                            {
                                SkipPastCurrentToken(state);
                                
                                struct_member->is_using = true;
                            }
                            
                            token = GetToken(state);
                            if (token.kind != Token_Identifier)
                            {
                                //// ERROR: Missing struct member name
                                encountered_errors = true;
                            }
                            
                            else if (token.keyword != Keyword_Invalid)
                            {
                                //// ERROR: Invalid use of keyword as struct member name
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                struct_member->name = token.identifier;
                                
                                SkipPastCurrentToken(state);
                                token = GetToken(state);
                                
                                if (token.kind == Token_Comma)
                                {
                                    //// ERROR: Multivariable declarations are not allowed in structs
                                    encountered_errors = true;
                                }
                                
                                else if (token.kind != Token_Colon)
                                {
                                    //// ERROR: Missing type of struct member
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    SkipPastCurrentToken(state);
                                    
                                    if (!ParseExpression(state, &struct_member->type)) encountered_errors = true;
                                    else
                                    {
                                        token = GetToken(state);
                                        
                                        if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                                        else break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing matching closing brace
                            encountered_errors = true;
                        }
                    }
                }
            }
        }
        
        else if (token.keyword == Keyword_Enum)
        {
            Enum_Expression* enum_expr = PushExpression(state, Expr_Enum);
            *expression = (Expression*)enum_expr;
            
            SkipPastCurrentToken(state);
            token = GetToken(state);
            
            if (token.kind != Token_OpenBrace)
            {
                if (!ParseExpression(state, &enum_expr->elem_type))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                token = GetToken(state);
                
                if (token.kind == Token_TripleMinus)
                {
                    SkipPastCurrentToken(state);
                    
                    enum_expr->is_decl = true;
                }
                
                else if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
                {
                    //// ERROR: Do is not applicable on enums
                    encountered_errors = true;
                }
                
                else if (token.kind != Token_OpenBrace)
                {
                    //// ERROR: Missing enum body
                    encountered_errors = true;
                }
                
                else
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind != Token_CloseBrace)
                    {
                        Enum_Member* prev_member = 0;
                        while (!encountered_errors)
                        {
                            Enum_Member* enum_member = Arena_PushSize(state.arena, sizeof(Enum_Member), ALIGNOF(Enum_Member));
                            ZeroStruct(enum_member);
                            
                            if (prev_member == 0) enum_expr->members = enum_member;
                            else                  prev_member->next  = enum_member;
                            prev_member = enum_member;
                            
                            token = GetToken(state);
                            
                            if (token.kind != Token_Identifier)
                            {
                                //// ERROR: Missing enum member name
                                encountered_errors = true;
                            }
                            
                            else if (token.keyword != Keyword_Invalid)
                            {
                                //// ERROR: Invalid use of keyword as enum member
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                enum_member->name = token.identifier;
                                
                                SkipPastCurrentToken(state);
                                token = GetToken(state);
                                
                                if (token.kind == Token_Equals)
                                {
                                    SkipPastCurrentToken(state);
                                    
                                    if (!ParseExpression(state, &enum_member->value))
                                    {
                                        encountered_errors = true;
                                    }
                                }
                                
                                if (!encountered_errors)
                                {
                                    token = GetToken(state);
                                    
                                    if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                                    else break;
                                }
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing matching closing brace
                            encountered_errors = true;
                        }
                    }
                }
            }
        }
        
        else
        {
            //// ERROR: Invalid use of keyword in expression
            encountered_errors = true;
        }
    }
    
    else if (token.kind == Token_Period)
    {
        SkipPastCurrentToken(state);
        token = GetToken(state);
        
        if (token.kind == Token_OpenBrace)
        {
            SkipPastCurrentToken(state);
            
            *expression = PushExpression(state, Expr_StructLiteral);
            ((StructLiteral_Expression*)*expression)->type = 0;
            
            if (!ParseArgumentList(state, &((StructLiteral_Expression*)*expression)->arguments)) encountered_errors = true;
            else
            {
                token = GetToken(state);
                
                if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                else
                {
                    //// ERROR: Missing matching closing brace
                    encountered_errors = true;
                }
            }
        }
        
        else if (token.kind == Token_OpenBracket)
        {
            SkipPastCurrentToken(state);
            
            *expression = PushExpression(state, Expr_ArrayLiteral);
            ((ArrayLiteral_Expression*)*expression)->type = 0;
            
            if (!ParseArgumentList(state, &((ArrayLiteral_Expression*)*expression)->arguments)) encountered_errors = true;
            else
            {
                token = GetToken(state);
                
                if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                else
                {
                    //// ERROR: Missing matching closing brace
                    encountered_errors = true;
                }
            }
        }
        
        else if (token.kind == Token_OpenParen)
        {
            SkipPastCurrentToken(state);
            
            if (!ParseExpression(state, expression)) encountered_errors = true;
            else
            {
                token = GetToken(state);
                
                if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
                else
                {
                    //// ERROR: Missing matching closing parenthesis
                    encountered_errors = true;
                }
            }
        }
        
        else if (token.kind == Token_Pound)
        {
            Directive_Expression directive_expr = PushExpression(state, Expr_Directive);
            *expression = (Expression*)directive_expr;
            
            SkipPastCurrentToken(state);
            token = GetToken(state);
            
            if (token.kind != Token_Identifier)
            {
                //// ERROR: Missing name of directive
                encountered_errors = true;
            }
            
            else if (token.identifier == BLANK_IDENTIFIER)
            {
                //// ERROR: Invalid use of blank identifier as directive name
                encountered_errors = true;
            }
            
            else
            {
                directive_expr->name = token.identifier;
                
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                if (token.kind == Token_OpenParen)
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    if (token.kind != Token_CloseParen)
                    {
                        if (!ParseArgumentList(state, &attribute->arguments))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing closing parenthesis
                            encountered_errors = true;
                        }
                    }
                }
            }
        }
        
        else
        {
            if (token.kind != Token_Identifier)
            {
                //// ERROR: Missing element name on right hand side of 'element of' operator
                encountered_errors = true;
            }
            
            else if (token.keyword != Keyword_Invalid)
            {
                //// ERROR: Invalid use of keyword as element name in 'element of' expression
                encountered_errors = true;
            }
            
            else
            {
                ElementOf_Expression* elementof_expr = PushExpression(state, Expr_ElementOf);
                elementof_expr->left  = 0;
                elementof_expr->right = token.identifier;
                
                *expression = (Expression*)elementof_expr;
                
                SkipPastCurrentToken(state);
            }
        }
    }
    
    else
    {
        //// ERROR: Missing primary expression
        encountered_errors = true;
    }
    
    return !encountered_errors;
}

bool
ParseTypeLevelExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_Hat || token.kind == Token_Cash)
    {
        *expression = PushExpression(state, token.kind == Token_Hat ? Expr_PointerType : Expr_PolymorphicType);
        
        SkipPastCurrentToken(state);
        
        if (!ParseTypeLevelExpression(state, (Expression**)&((Unary_Expression*)*expression)->operand))
        {
            encountered_errors = true;
        }
    }
    
    else if (token.kind == Token_OpenBracket)
    {
        SkipPastCurrentToken(state);
        token      = GetToken(state);
        Token peek = PeekToken(state);
        
        Expression** operand;
        if (token.kind == Token_CloseBracket)
        {
            SkipPastCurrentToken(state);
            
            *expression = PushExpression(state, Expr_SliceType);
            operand = &((Unary_Expression*)*expression)->operand;
        }
        
        else if (token.kind == Token_Elipsis && peek.kind == Token_CloseBracket)
        {
            SkipPastCurrentToken(state);
            SkipPastCurrentToken(state);
            
            *expression = PushExpression(state, Expr_DynamicArrayType);
            operand = &((Unary_Expression*)*expression)->operand;
        }
        
        else
        {
            *expression = PushExpression(state, Expr_ArrayType);
            operand = &((ArrayType_Expression*)*expression)->type;
            
            if (!ParseExpression(state, (Expression**)&((ArrayType_Expression*)*expression)->size))
            {
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors)
        {
            if (!ParseTypeLevelExpression(state, operand))
            {
                encountered_errors = true;
            }
        }
    }
    
    else
    {
        if (!ParsePrimaryExpression(state, expression))
        {
            encountered_errors = true;
        }
    }
    
    return !encountered_errors;
}

bool
ParsePostfixExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    if (!ParseTypeLevelExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Token token = GetToken(state);
            
            if (token.kind == Token_OpenBracket)
            {
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                Expression* array = *expression;
                Expression* index = 0;
                
                if (token.kind != Token_Colon && !ParseExpression(state, &index)) encountered_errors = true;
                else
                {
                    token = GetToken(state);
                    
                    if (token.kind == Token_CloseBracket)
                    {
                        *expression = PushExpression(state, Expr_Subscript);
                        ((Subscript_Expression*)*expression)->array = array;
                        ((Subscript_Expression*)*expression)->index = index;
                    }
                    
                    else if (token.kind == Token_Colon)
                    {
                        SkipPastCurrentToken(state);
                        token = GetToken(state);
                        
                        Expression* end = 0;
                        if (token.kind != Token_CloseBracket)
                        {
                            if (!ParseExpression(state, &end))
                            {
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            *expression = PushExpression(state, Expr_Slice);
                            ((Slice_Expression*)*expression)->array = array;
                            ((Slice_Expression*)*expression)->start = index;
                            ((Slice_Expression*)*expression)->end   = end;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseBracket) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing matching closing bracket
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            else if (token.kind == Token_OpenParen)
            {
                SkipPastCurrentToken(state);
                
                Expression* pointer = *expression;
                
                *expression = PushExpression(state, Expr_Call);
                ((Call_Expression*)*expression)->pointer = pointer;
                
                if (!ParseArgumentList(state, &((Call_Expression*)*expression)->arguments)) encountered_errors = true;
                else
                {
                    token = GetToken(state);
                    
                    if (token.kind == Token_CloseParen) SkipPastCurrentToken(state);
                    else
                    {
                        //// ERROR: Missing matching closing parenthesis
                        encountered_errors = true;
                    }
                }
            }
            
            else if (token.kind == Token_Period)
            {
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                if (token.kind == Token_OpenBrace)
                {
                    SkipPastCurrentToken(state);
                    
                    Expression* type = *expression;
                    
                    *expression = PushExpression(state, Expr_StructLiteral);
                    ((StructLiteral_Expression*)*expression)->type = type;
                    
                    if (!ParseArgumentList(state, &((StructLiteral_Expression*)*expression)->arguments)) encountered_errors = true;
                    else
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing matching closing brace
                            encountered_errors = true;
                        }
                    }
                }
                
                else if (token.kind == Token_OpenBracket)
                {
                    SkipPastCurrentToken(state);
                    
                    Expression* type = *expression;
                    
                    *expression = PushExpression(state, Expr_ArrayLiteral);
                    ((ArrayLiteral_Expression*)*expression)->type = type;
                    
                    if (!ParseArgumentList(state, &((ArrayLiteral_Expression*)*expression)->arguments)) encountered_errors = true;
                    else
                    {
                        token = GetToken(state);
                        
                        if (token.kind == Token_CloseBrace) SkipPastCurrentToken(state);
                        else
                        {
                            //// ERROR: Missing matching closing brace
                            encountered_errors = true;
                        }
                    }
                }
                
                else
                {
                    if (token.kind != Token_Identifier)
                    {
                        //// ERROR: Missing element name on right hand side of 'element of' operator
                        encountered_errors = true;
                    }
                    
                    else if (token.keyword != Keyword_Invalid)
                    {
                        //// ERROR: Invalid use of keyword as element name in 'element of' expression
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        ElementOf_Expression* elementof_expr = PushExpression(state, Expr_ElementOf);
                        elementof_expr->left  = *expression;
                        elementof_expr->right = token.identifier;
                        
                        *expression = (Expression*)elementof_expr;
                        
                        SkipPastCurrentToken(state);
                    }
                }
            }
            
            else break;
        }
    }
    
    return !encountered_errors;
}

bool
ParsePrefixExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_Plus) SkipPastCurrentToken(state);
    else
    {
        Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
        
        switch (token.kind)
        {
            case Token_Minus:      kind = Expr_Negation;        break;
            case Token_Complement: kind = Expr_Complement;      break;
            case Token_Not:        kind = Expr_Not;             break;
            case Token_Increment:  kind = Expr_PreIncrement;    break;
            case Token_Decrement:  kind = Expr_PreDecrement;    break;
            case Token_And:        kind = Expr_Reference;       break;
            case Token_Star:       kind = Expr_Dereference;     break;
            case Token_Elipsis:    kind = Expr_Spread;          break;
        }
        
        if (kind != Expr_Invalid)
        {
            SkipPastCurrentToken(state);
            
            Unary_Expression* unary_expr = PushExpression(state, kind);
            *expression = (Expression*)unary_expr;
            
            if (!ParsePrefixExpression(state, &unary_expr->operand))
            {
                encountered_errors = true;
            }
        }
        
        else
        {
            if (!ParsePostfixExpression(state, expression))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseBinaryExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrefixExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Token token = GetToken(state);
            
            // IMPORTANT NOTE: EXPRESSION_KIND is organized in blocks of values, each 20 in size
            //                 the blocks from 4 to 9 contain binary expressions
            
            Enum8(EXPRESSION_KIND) op = (token.kind == Token_Identifier && token.keyword == Keyword_Invalid
                                         ? Expr_InfixCall
                                         : token.kind);
            umm precedence = op / 20;
            
            // precedence < 4 || precedence > 9
            if (precedence - 4 > 5) break;
            else
            {
                SkipPastCurrentToken(state);
                
                Binary_Expression* binary_expr = PushExpression(state, op);
                
                if (!ParsePrefixExpression(state, &binary_expr->right)) encountered_errors = true;
                else
                {
                    Expression** slot = expression;
                    
                    for (;;)
                    {
                        if ((*slot)->kind / 20 <= precedence)
                        {
                            binary_expr->left = *slot;
                            *slot             = (Expression*)binary_expr;
                        }
                        
                        else
                        {
                            slot = (Expression**)&((Binary_Expression*)*slot)->right;
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseConditional(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    if (!ParseBinaryExpression(state, expression)) encountered_errors = true;
    else
    {
        Token token = GetToken(state);
        if (token.kind == Token_QuestionMark)
        {
            SkipPastCurrentToken(state);
            
            Conditional_Expression* conditional_expr = PushExpression(state, Expr_Conditional);
            conditional_expr->condition = *expression;
            *expression                 = (Expression*)conditional_expr;
            
            if (!ParseBinaryExpression(state, &conditional_expr->true_branch)) encountered_errors = true;
            else
            {
                token = GetToken(state);
                if (token.kind != Token_Colon)
                {
                    //// ERROR: Missing false branch
                    encountered_errors = true;
                }
                
                else
                {
                    SkipPastCurrentToken(state);
                    
                    if (!ParseBinaryExpression(state, &conditional_expr->false_branch))
                    {
                        encountered_errors = true;
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseExpression(Parser_State state, Expression** expression)
{
    return ParseBinaryExpression(state, expression);
}

bool
ParseStatement(Parser_State state, Statement** statement, Any_Statement* memory)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    Token peek  = PeekToken(state);
    
    ASSERT(token.kind != Token_Semicolon);
    
    if (token.kind == Token_EndOfStream)
    {
        //// ERROR: Expected statement but got end of stream
        encountered_errors = true;
    }
    
    else if (token.kind == Token_Identifier &&
             token.keyword >= Keyword_FirstStatementInitiator && token.keyword <= Keyword_LastStatementInitiator &&
             (token.keyword != Keyword_Using || peek.keyword == Keyword_Import || peek.keyword == Keyword_Foreign))
    {
        if (token.keyword == Keyword_Using || token.keyword == Keyword_Import || token.keyword == Keyword_Foreign)
        {
            NOT_IMPLEMENTED;
        }
        
        else
        {
            if (token.keyword == Keyword_If || token.keyword == Keyword_When)
            {
                Enum8(STATEMENT_KIND) kind = (token.keyword == Keyword_If ? Statement_If : Statement_When);
                If_Statement* if_statement = PushStatement(state, kind, memory);
                *statement = (Statement*)if_statement;
                
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                Statement* first      = 0;
                Any_Statement backing = {0};
                if (token.kind != Token_Semicolon)
                {
                    if (!ParseStatement(state, &first, &backing))
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(state);
                    if (token.kind == Token_Semicolon)
                    {
                        SkipPastCurrentToken(state);
                        
                        if_statement->init = PushStatement(state, first->kind, 0);
                        Copy(first, if_statement->init, Sizeof_Statement(first->kind));
                        
                        if (!ParseExpression(state, &if_statement->condition))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    else
                    {
                        if (first->kind != Statement_Expression)
                        {
                            //// ERROR: Missing condition
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            if_statement->init      = 0;
                            if_statement->condition = ((Expression_Statement*)first)->expression;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        if (token.kind != Token_OpenBrace && (token.kind != Token_Identifier || token.keyword != Keyword_Do))
                        {
                            //// ERROR: Missing body
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            if (!ParseBlockStatement(state, &if_statement->true_branch)) encountered_errors = true;
                            else
                            {
                                token = GetToken(state);
                                
                                if (token.kind == Token_Identifier && token.keyword == Keyword_Else)
                                {
                                    SkipPastCurrentToken(state);
                                    token = GetToken(state);
                                    
                                    if (token.kind == Token_Identifier && token.keyword == Keyword_If)
                                    {
                                        if_statement->false_branch.kind = Statement_Block;
                                        if (!ParseStatement(state, &if_statement->false_branch.statements, 0))
                                        {
                                            encountered_errors = true;
                                        }
                                    }
                                    
                                    else
                                    {
                                        if (!ParseBlockStatement(state, &if_statement->false_branch))
                                        {
                                            encountered_errors = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            else if (token.keyword == Keyword_Else)
            {
                //// ERROR: Else without matching if/when
                encountered_errors = true;
            }
            
            else if (token.keyword == Keyword_While)
            {
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                While_Statement* while_statement = PushStatement(state, Statement_While, memory);
                *statement = (Statement*)while_statement;
                
                Statement* first            = 0;
                Any_Statement first_backing = {0};
                if (token.kind != Token_Semicolon)
                {
                    if (!ParseStatement(state, &first, &first_backing))
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(state);
                    if (token.kind != Token_Semicolon)
                    {
                        if (first->kind == Statement_Expression) while_statement->condition = ((Expression_Statement*)first)->expression;
                        else
                        {
                            //// ERROR: Missing condition
                            encountered_errors = true;
                        }
                    }
                    
                    else
                    {
                        Statement* second            = 0;
                        Any_Statement second_backing = {0};
                        
                        SkipPastCurrentToken(state);
                        token = GetToken(state);
                        
                        if (token.kind != Token_Semicolon)
                        {
                            if (!ParseStatement(state, &second, &second_backing))
                            {
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            if (second->kind != Statement_Expression)
                            {
                                //// ERROR: Missing condition, expected init; condition, but condition was a statement
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                while_statement->init = PushStatement(state, first->kind, 0);
                                Copy(first, while_statement->init, Sizeof_Statement(first->kind));
                                
                                while_statement->condition = ((Expression_Statement*)second)->expression;
                                
                                token = GetToken(state);
                                if (token.kind == Token_Semicolon)
                                {
                                    if (!ParseStatement(state, &while_statement->step, 0))
                                    {
                                        encountered_errors = true;
                                    }
                                }
                                
                                if (!encountered_errors)
                                {
                                    token = GetToken(state);
                                    if (token.kind != Token_OpenBrace && (token.kind != Token_Identifier || token.keyword != Keyword_Do))
                                    {
                                        //// ERROR: Missing body
                                        encountered_errors = true;
                                    }
                                    
                                    else
                                    {
                                        if (!ParseBlockStatement(state, &while_statement->body))
                                        {
                                            encountered_errors = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            else if (token.keyword == Keyword_For)
            {
                SkipPastCurrentToken(state);
                
                For_Statement* for_statement = PushStatement(state, Statement_For, memory);
                *statement = (Statement*)for_statement;
                
                Expression* expressions = 0;
                Expression* prev_expr   = 0;
                while (!encountered_errors)
                {
                    Expression* expression;
                    if (!ParseExpression(state, &expression)) encountered_errors = true;
                    else
                    {
                        if (prev_expr == 0) expressions     = expression;
                        else                prev_expr->next = expression;
                        prev_expr = expression;
                        
                        token = GetToken(state);
                        if (token.kind == Token_Semicolon) SkipPastCurrentToken(state);
                        else break;
                    }
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(state);
                    if (token.kind != Token_Identifier || token.keyword != Keyword_In)
                    {
                        if (expressions->next == 0) for_statement->collection = expressions;
                        else
                        {
                            //// ERROR: Cannot iterate over several collections (maybe missing 'in')
                            encountered_errors = true;
                        }
                    }
                    
                    else
                    {
                        for_statement->symbols = expressions;
                        
                        SkipPastCurrentToken(state);
                        
                        if (!ParseExpression(state, &for_statement->collection))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        if (token.kind != Token_OpenBrace && (token.kind != Token_Identifier || token.keyword != Keyword_Do))
                        {
                            //// ERROR: Missing body
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            if (!ParseBlockStatement(state, &for_statement->body))
                            {
                                encountered_errors = true;
                            }
                        }
                    }
                }
            }
            
            else if (token.keyword == Keyword_Break || token.keyword == Keyword_Continue)
            {
                Enum8(STATEMENT_KIND) kind = (token.keyword == Keyword_Break ? Statement_Break : Statement_Continue);
                ScopeControl_Statement* scopecontrol_statement = PushStatement(state, kind, memory);
                *statement = (Statement*)scopecontrol_statement;
                
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                if (token.kind == Token_Identifier)
                {
                    if (token.keyword != Keyword_Invalid)
                    {
                        //// ERROR: Invalid use of keyword as label
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        scopecontrol_statement->label = token.identifier;
                        SkipPastCurrentToken(state);
                    }
                }
            }
            
            else if (token.keyword == Keyword_Defer)
            {
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                if (token.kind == Token_Semicolon)
                {
                    //// ERROR: Missing statement after defer
                    encountered_errors = true;
                }
                
                else
                {
                    Defer_Statement* defer_statement = PushStatement(state, Statement_Defer, memory);
                    *statement = (Statement*)defer_statement;
                    
                    if (!ParseStatement(state, &defer_statement->statement, 0))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            else if (token.keyword == Keyword_Return)
            {
                SkipPastCurrentToken(state);
                token = GetToken(state);
                
                Return_Statement* return_statement = PushStatement(state, Statement_Return, memory);
                *statement = (Statement*)return_statement;
                
                if (token.kind != Token_Semicolon)
                {
                    if (!ParseArgumentList(state, &return_statement->arguments))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            else if (token.keyword == Keyword_Unreachable)
            {
                *statement = PushStatement(state, Statement_Unreachable, memory);
                SkipPastCurrentToken(state);
            }
            
            else if (token.keyword == Keyword_NotImplemented)
            {
                *statement = PushStatement(state, Statement_NotImplemented, memory);
                SkipPastCurrentToken(state);
            }
            
            else INVALID_CODE_PATH;
        }
    }
    
    else
    {
        Token peek_next = PeekNextToken(state);
        
        if ((token.kind == Token_Identifier || token.kind == Token_Underscore) && peek.kind == Token_Colon &&
            (peek_next.kind == Token_OpenBrace ||
             peek_next.kind == Token_Identifier &&
             peek_next.keyword >= Keyword_FirstStatementInitiator && peek_next.keyword <= Keyword_LastStatementInitiator))
        {
            Identifier label = BLANK_IDENTIFIER;
            
            if (token.kind == Token_Identifier)
            {
                if (token.keyword == Keyword_Invalid) label = token.identifier;
                else
                {
                    //// ERROR: Invalid use of keyword as label name
                    encountered_errors = true;
                }
            }
            
            else if (peek_next.kind != Token_OpenBrace && peek_next.keyword != Keyword_If &&
                     peek_next.keyword != Keyword_When && peek_next.keyword != Keyword_While &&
                     peek_next.keyword != Keyword_For)
            {
                //// ERROR: Labels are only applicable to braced blocks, if, when and for statements
                encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                SkipPastCurrentToken(state);
                SkipPastCurrentToken(state);
                
                if (!ParseStatement(state, statement, 0)) encountered_errors = true;
                else
                {
                    if      ((*statement)->kind == Statement_Block) ((Block_Statement*)*statement)->label = label;
                    else if ((*statement)->kind == Statement_If)    ((If_Statement*)*statement)->label    = label;
                    else if ((*statement)->kind == Statement_When)  ((If_Statement*)*statement)->label    = label;
                    else if ((*statement)->kind == Statement_While) ((While_Statement*)*statement)->label = label;
                    else                                         ((For_Statement*)*statement)->label   = label;
                }
            }
        }
        
        else
        {
            bool is_using = false;
            if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
            {
                is_using = true;
                SkipPastCurrentToken(state);
            }
            
            Expression* expressions = 0;
            Expression* prev_expr   = 0;
            while (!encountered_errors)
            {
                Expression* expression;
                if (!ParseExpression(state, &expression)) encountered_errors = true;
                else
                {
                    if (prev_expr == 0) expressions     = expression;
                    else                prev_expr->next = expression;
                    prev_expr = expression;
                    
                    token = GetToken(state);
                    if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                    else break;
                }
            }
            
            if (!encountered_errors)
            {
                token = GetToken(state);
                if (token.kind == Token_Colon)
                {
                    SkipPastCurrentToken(state);
                    token = GetToken(state);
                    
                    Expression* names  = expressions;
                    Expression* type   = 0;
                    Expression* values = 0;
                    
                    if (token.kind != Token_Colon && token.kind != Token_Equals)
                    {
                        if (!ParseExpression(state, &type))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        token = GetToken(state);
                        
                        bool is_constant      = false;
                        bool is_uninitialized = false;
                        if (token.kind == Token_Colon || token.kind == Token_Equals)
                        {
                            is_constant = (token.kind == Token_Colon);
                            
                            SkipPastCurrentToken(state);
                            token = GetToken(state);
                            
                            if (token.kind == Token_TripleMinus)
                            {
                                if (is_constant)
                                {
                                    //// ERROR: Constants cannot be left uninitialized
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    is_uninitialized = true;
                                    SkipPastCurrentToken(state);
                                }
                            }
                            
                            else
                            {
                                prev_expr = 0;
                                while (!encountered_errors)
                                {
                                    Expression* expression;
                                    if (!ParseExpression(state, &expression)) encountered_errors = true;
                                    else
                                    {
                                        if (prev_expr == 0) values          = expression;
                                        else                prev_expr->next = expression;
                                        prev_expr = expression;
                                        
                                        token = GetToken(state);
                                        if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                                        else break;
                                    }
                                }
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            if (is_constant)
                            {
                                Constant_Declaration* const_decl = PushStatement(state, Statement_ConstantDecl, memory);
                                *statement = (Statement*)const_decl;
                                
                                const_decl->names    = names;
                                const_decl->type     = type;
                                const_decl->values   = values;
                                const_decl->is_using = is_using;
                            }
                            
                            else
                            {
                                Variable_Declaration* var_decl = PushStatement(state, Statement_VariableDecl, memory);
                                *statement = (Statement*)var_decl;
                                
                                var_decl->names            = names;
                                var_decl->type             = type;
                                var_decl->values           = values;
                                var_decl->is_uninitialized = is_uninitialized;
                                var_decl->is_using         = is_using;
                            }
                        }
                    }
                    
                }
                
                else if (token.kind >= Token_FirstAssignment && token.kind <= Token_LastAssignment)
                {
                    if (is_using)
                    {
                        //// ERROR: Using is not applicable on assignment statements
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        Assignment_Statement* assignment_statement = PushStatement(state, Statement_Assignment, memory);
                        *statement = (Statement*)assignment_statement;
                        
                        assignment_statement->lhs = expressions;
                        
                        switch (token.kind)
                        {
                            case Token_OrOrEquals:                 assignment_statement->op = Expr_Or;                   break;
                            case Token_AndAndEquals:               assignment_statement->op = Expr_And;                  break;
                            case Token_PlusEquals:                 assignment_statement->op = Expr_Add;                  break;
                            case Token_MinusEquals:                assignment_statement->op = Expr_Sub;                  break;
                            case Token_OrEquals:                   assignment_statement->op = Expr_BitwiseOr;            break;
                            case Token_HatEquals:                  assignment_statement->op = Expr_BitwiseXor;           break;
                            case Token_StarEquals:                 assignment_statement->op = Expr_Mul;                  break;
                            case Token_SlashEquals:                assignment_statement->op = Expr_Div;                  break;
                            case Token_RemEquals:                  assignment_statement->op = Expr_Rem;                  break;
                            case Token_ModEquals:                  assignment_statement->op = Expr_Mod;                  break;
                            case Token_AndEquals:                  assignment_statement->op = Expr_BitwiseAnd;           break;
                            case Token_ArithmeticRightShiftEquals: assignment_statement->op = Expr_ArithmeticRightShift; break;
                            case Token_RightShiftEquals:           assignment_statement->op = Expr_RightShift;           break;
                            case Token_LeftShiftEquals:            assignment_statement->op = Expr_LeftShift;            break;
                        }
                        
                        prev_expr = 0;
                        while (!encountered_errors)
                        {
                            Expression* expression;
                            if (!ParseExpression(state, &expression)) encountered_errors = true;
                            else
                            {
                                if (prev_expr == 0) assignment_statement->rhs = expression;
                                else                prev_expr->next           = expression;
                                prev_expr = expression;
                                
                                token = GetToken(state);
                                if (token.kind == Token_Comma) SkipPastCurrentToken(state);
                                else break;
                            }
                        }
                    }
                }
                
                else
                {
                    if (expressions->next != 0)
                    {
                        //// ERROR: Sequence expressions are not a thing in Niob
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        if (is_using)
                        {
                            Using_Statement* using_statement = PushStatement(state, Statement_Using, memory);
                            *statement = (Statement*)using_statement;
                            
                            using_statement->symbol = expressions;
                        }
                        
                        else
                        {
                            Expression_Statement* expr_statement = PushStatement(state, Statement_Expression, memory);
                            *statement = (Statement*)expr_statement;
                            
                            expr_statement->expression = expressions;
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseBlockStatement(Parser_State state, Block_Statement* block)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
    {
        block->is_do = true;
    }
    
    else ASSERT(token.kind == Token_OpenBrace);
    
    SkipPastCurrentToken(state);
    
    Statement* prev_statement = 0;
    while (!encountered_errors)
    {
        token = GetToken(state);
        
        if (token.kind == Token_CloseBrace)
        {
            if (block->is_do)
            {
                //// ERROR: Missing statement after 'do'
                encountered_errors = true;
            }
            
            else
            {
                SkipPastCurrentToken(state);
                break;
            }
        }
        
        else
        {
            token = GetToken(state);
            
            if (token.kind == Token_Semicolon) SkipPastCurrentToken(state);
            else
            {
                Statement* statement = 0;
                if (!ParseStatement(state, &statement, 0)) encountered_errors = true;
                else
                {
                    if (prev_statement == 0) block->statements = statement;
                    else                     prev_statement->next = statement;
                    prev_statement = statement;
                    
                    token = GetToken(state);
                    if (token.kind == Token_Semicolon) SkipPastCurrentToken(state);
                    else
                    {
                        //// ERROR: Missing terminating semicolon after statement
                        encountered_errors = true;
                    }
                    
                    if (block->is_do) break;
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseText(Workspace* workspace, String text, Memory_Arena* ast_arena, Memory_Arena* token_arena, Memory_Arena* string_arena)
{
    bool encountered_errors = false;
    
    Bucket_Array(Token) tokens;
    
    Parser_State state = {0};
    state.arena  = ast_arena;
    state.tokens = &tokens;
    
    if (!LexText(workspace, text, token_arena, string_arena, &tokens)) encountered_errors = true;
    else
    {
        state.current_token = BA_Iterate(state.tokens, 0);
        
        while (!encountered_errors)
        {
            Token token = GetToken(state);
            
            if      (token.kind == Token_EndOfStream) break;
            else if (token.kind == Token_Semicolon) SkipPastCurrentToken(state);
            else
            {
                NOT_IMPLEMENTED;
                // TODO: wrap statement in declaration
            }
        }
    }
    
    return !encountered_errors;
}