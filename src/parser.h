typedef struct Parser_State
{
    Lexer* lexer;
    Memory_Arena* arena;
} Parser_State;

Token
GetToken(Parser_State state)
{
    return state.lexer->buffer[state.lexer->token_window_index];
}

Token
PeekToken(Parser_State state)
{
    return state.lexer->buffer[state.lexer->token_window_index + 1];
}

Token
PeekNextToken(Parser_State state)
{
    return state.lexer->buffer[state.lexer->token_window_index + 2];
}

void
SkipPastCurrentToken(Parser_State state)
{
    Lexer_AdvanceTokenWindow(state.lexer);
}

void*
PushExpression(Parser_State state, Enum8(EXPRESSION_KIND) kind)
{
    void* result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

bool ParseExpression(Parser_State state, Expression** expression);

bool
ParseArgumentList(Parser_State state, Argument** arguments)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

bool
ParseParameterList(Parser_State state, Parameter** parameters)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_OpenParen)
    {
        SkipPastCurrentToken(state);
        token = GetToken(state);
        
        if (token.kind != Token_CloseParen)
        {
            NOT_IMPLEMENTED;
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
            
            if (token.kind == Token_OpenParen)
            {
                if (!ParseParameterList(state, &proc_expr->parameters))
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
                            Return_Value* return_value = Arena_PushSize(state.arena, sizeof(Return_Value), ALIGNOF(Return_Value));
                            ZeroStruct(return_value);
                            
                            if (prev_value == 0) proc_expr->return_values = return_value;
                            else                 prev_value->next         = return_value;
                            prev_value = return_value;
                            
                            NOT_IMPLEMENTED;
                        }
                    }
                    
                    else
                    {
                        Return_Value* return_value = Arena_PushSize(state.arena, sizeof(Return_Value), ALIGNOF(Return_Value));
                        ZeroStruct(return_value);
                        
                        proc_expr->return_values = return_value;
                        
                        if (!ParseExpression(state, &return_value->type))
                        {
                            encountered_errors = true;
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
                            NOT_IMPLEMENTED;
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
                if (!ParseParameterList(state, &struct_expr->parameters)) encountered_errors = true;
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
                
                if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
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
                
                if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
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
            
            // IMPORTANT NOTE: EXPRESSION_KIND is organized in blocks of values, each 13 in size
            //                 the blocks from 3 to 8 contain binary expressions
            
            Enum8(EXPRESSION_KIND) op = (token.kind == Token_Identifier && token.keyword == Keyword_Invalid
                                         ? Expr_InfixCall
                                         : token.kind);
            umm precedence = op / 13;
            
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
                        if ((*slot)->kind / 13 <= precedence)
                        {
                            binary_expr->left = *slot;
                            *slot             = (Expression*)binary_expr;
                        }
                        
                        else
                        {
                            slot = &(Expression*)((Binary_Expression*)*slot)->right;
                        }
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
ParseText(Workspace* workspace, String string)
{
    bool encountered_errors = false;
    
    Lexer lexer = Lexer_Init(workspace, string);
    
    Parser_State state = {0};
    state.lexer = &lexer;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}