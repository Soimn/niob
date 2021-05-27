typedef struct Parser_State
{
    Memory_Arena* ast_arena;
    Memory_Arena* string_arena;
    
    u32 offset_to_end_of_last_token;
    Token* current_token;
} Parser_State;

enum SKIP_MODE
{
    SKIP_CURRENT,
    SKIP_PEEK,
};

void
SkipPastToken(Parser_State state, Enum8(SKIP_MODE) skip_mode)
{
    ASSERT(state.current_token->next != 0);
    
    state.offset_to_end_of_last_token = state.current_token->text.pos.offset + state.current_token->text.length;
    state.current_token               = state.current_token->next;
}

Enum32(TOKEN_KIND)
CurrentTokenKind(Parser_State state)
{
    return state.current_token->kind;
}

Enum32(TOKEN_KIND)
PeekTokenKind(Parser_State state)
{
    return state.current_token->next->kind;
}

bool
IsCurrentToken(Parser_State state, Enum32(TOKEN_KIND) kind)
{
    return (state.current_token->kind == kind);
}

bool
IsPeekToken(Parser_State state, Enum32(TOKEN_KIND) kind)
{
    return (state.current_token->next->kind == kind);
}

Expression*
PushExpression(Parser_State state, Enum8(EXPRESSION_KIND) kind)
{
    Expression* result = Arena_PushSize(state.ast_arena, sizeof(Expression), ALIGNOF(Expression));
    ZeroStruct(result);
    
    result->kind = kind;
    
    return result;
}

Text_Pos
CurrentTextPos(Parser_State state)
{
    return state.current_token->text.pos;
}

Text
TextSincePos(Parser_State state, Text_Pos pos)
{
    return (Text){.pos = pos, .length = state.offset_to_end_of_last_token - pos.offset};
}

bool ParseExpression(Parser_State state, Expression** expression);

bool
ParseType(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

bool
ParsePrimaryExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

bool
ParsePostfixExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrimaryExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            if (IsCurrentToken(state, Token_OpenBracket))
            {
                SkipPastToken(state, SKIP_CURRENT);
                
                Expression* pointer = *expression;
                Expression* index   = 0;
                
                if (!ParseExpression(state, &index)) encountered_errors = true;
                else
                {
                    if (IsCurrentToken(state, Token_Colon))
                    {
                        SkipPastToken(state, SKIP_CURRENT);
                        
                        *expression = PushExpression(state, Expr_Slice);
                        (*expression)->slice.pointer     = pointer;
                        (*expression)->slice.start_index = index;
                        
                        if (!ParseExpression(state, &(*expression)->slice.end_index))
                        {
                            encountered_errors = true;
                            break;
                        }
                    }
                    
                    else
                    {
                        *expression = PushExpression(state, Expr_Subscript);
                        (*expression)->subscript.pointer = pointer;
                        (*expression)->subscript.index   = index;
                    }
                    
                    if (!IsCurrentToken(state, Token_CloseBracket))
                    {
                        //// ERROR: missing closing bracket
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        SkipPastToken(state, SKIP_CURRENT);
                        (*expression)->text = TextSincePos(state, (*expression)->slice.pointer->text.pos);
                    }
                }
            }
            
            else if (IsCurrentToken(state, Token_OpenParen))
            {
                SkipPastToken(state, SKIP_CURRENT);
                
                Expression* pointer = *expression;
                
                *expression = PushExpression(state, Expr_Call);
                (*expression)->call.pointer = pointer;
                
                Named_Argument* last_argument = 0;
                while (!encountered_errors)
                {
                    Named_Argument* argument = 0;
                    
                    if (last_argument == 0)
                    {
                        argument      = &(*expression)->call.arguments;
                        last_argument = argument;
                    }
                    
                    else
                    {
                        argument = Arena_PushSize(state.ast_arena, sizeof(Named_Argument), ALIGNOF(Named_Argument));
                        ZeroStruct(argument);
                        
                        last_argument->next = argument;
                        last_argument       = argument;
                    }
                    
                    if (!ParseExpression(state, &argument->value)) encountered_errors = true;
                    else
                    {
                        if (IsCurrentToken(state, Token_Equal))
                        {
                            SkipPastToken(state, SKIP_CURRENT);
                            
                            argument->name = argument->value;
                            if (!ParseExpression(state, &argument->value)) encountered_errors = true;
                        }
                        
                        if (!encountered_errors)
                        {
                            if (IsCurrentToken(state, Token_Comma)) SkipPastToken(state, SKIP_CURRENT);
                            else break;
                        }
                    }
                }
                
                if (!IsCurrentToken(state, Token_CloseParen))
                {
                    //// ERROR: Missing closing parenthesis
                    encountered_errors = true;
                }
                
                else
                {
                    SkipPastToken(state, SKIP_CURRENT);
                    
                    (*expression)->text = TextSincePos(state, (*expression)->call.pointer->text.pos);
                }
            }
            
            else if (IsCurrentToken(state, Token_Period))
            {
                Expression* left = *expression;
                
                *expression = PushExpression(state, Expr_Member);
                (*expression)->left = left;
                
                if (!ParsePrimaryExpression(state, &(*expression)->right)) encountered_errors = true;
                else (*expression)->text = TextSincePos(state, left->text.pos);
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
    
    Text_Pos start_text_pos = CurrentTextPos(state);
    
    Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
    
    switch (CurrentTokenKind(state))
    {
        case Token_Plus:   kind = Expr_Plus;        break;
        case Token_Minus:  kind = Expr_Minus;       break;
        case Token_BitAnd: kind = Expr_Reference;   break;
        case Token_Star:   kind = Expr_Dereference; break;
        case Token_BitNot: kind = Expr_BitNot;      break;
        case Token_Not:    kind = Expr_Not;         break;
    }
    
    if (kind == Expr_Invalid) encountered_errors = !ParsePostfixExpression(state, expression);
    else
    {
        *expression = PushExpression(state, kind);
        
        if (!ParsePrefixExpression(state, &(*expression)->operand)) encountered_errors = true;
        else
        {
            (*expression)->text = TextSincePos(state, start_text_pos);
        }
    }
    
    return !encountered_errors;
}

// NOTE: Does not parse member expressions
bool
ParseBinaryExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrefixExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
            umm precedence              = U64_MAX;
            
            switch (CurrentTokenKind(state))
            {
                case Token_Or:             kind = Expr_Or;          precedence = 1; break;
                case Token_And:            kind = Expr_And;         precedence = 2; break;
                case Token_EqualEqual:     kind = Expr_IsEqual;     precedence = 3; break;
                case Token_NotEqual:       kind = Expr_IsNotEqual;  precedence = 3; break;
                case Token_Less:           kind = Expr_IsLess;      precedence = 3; break;
                case Token_LessEqual:      kind = Expr_IsLessEQ;    precedence = 3; break;
                case Token_Greater:        kind = Expr_IsGreater;   precedence = 3; break;
                case Token_GreaterEqual:   kind = Expr_IsGreaterEQ; precedence = 3; break;
                case Token_Plus:           kind = Expr_Add;         precedence = 4; break;
                case Token_Minus:          kind = Expr_Sub;         precedence = 4; break;
                case Token_BitOr:          kind = Expr_BitOr;       precedence = 4; break;
                case Token_Hat:            kind = Expr_BitXor;      precedence = 4; break;
                case Token_Star:           kind = Expr_Mul;         precedence = 5; break;
                case Token_Slash:          kind = Expr_Div;         precedence = 5; break;
                case Token_Percentage:     kind = Expr_Rem;         precedence = 5; break;
                case Token_BitAnd:         kind = Expr_BitAnd;      precedence = 5; break;
                case Token_LessLess:       kind = Expr_LShift;      precedence = 5; break;
                case Token_GreaterGreater: kind = Expr_RShift;      precedence = 5; break;
                case Token_Identifier:     kind = Expr_InfixCall;   precedence = 5; break;
                case Token_Arrow:          kind = Expr_Chain;       precedence = 5; break;
            }
            
            if (kind == Expr_Invalid) break;
            else
            {
                SkipPastToken(state, SKIP_CURRENT);
                
                Expression* right;
                if (!ParsePrefixExpression(state, &right)) encountered_errors = true;
                else
                {
                    Expression** scan  = expression;
                    Expression* parent = 0;
                    
                    for (;;)
                    {
                        umm scan_precedence = U64_MAX;
                        
                        if      ((*scan)->kind == Expr_Or)                                           scan_precedence = 1;
                        else if ((*scan)->kind == Expr_And)                                          scan_precedence = 2;
                        else if ((*scan)->kind >= Expr_IsEqual && (*scan)->kind <= Expr_IsGreaterEQ) scan_precedence = 3;
                        else if ((*scan)->kind >= Expr_Add     && (*scan)->kind <= Expr_BitXor)      scan_precedence = 4;
                        else if ((*scan)->kind >= Expr_Mul     && (*scan)->kind <= Expr_Chain)       scan_precedence = 5;
                        
                        if (scan_precedence >= precedence)
                        {
                            Expression* new_expression = PushExpression(state, kind);
                            new_expression->text   = TextSincePos(state, (*scan)->text.pos);
                            new_expression->left   = *scan;
                            new_expression->right  = right;
                            
                            *scan = new_expression;
                            
                            if (parent != 0) parent->text = TextSincePos(state, parent->text.pos);
                            break;
                        }
                        
                        else
                        {
                            parent = *scan;
                            scan   = &(*scan)->right;
                            continue;
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
    bool encountered_errors = false;
    
    if (!ParseBinaryExpression(state, expression)) encountered_errors = true;
    else
    {
        if (IsCurrentToken(state, Token_Question))
        {
            SkipPastToken(state, SKIP_CURRENT);
            
            Expression* condition  = *expression;
            Expression* true_expr  = 0;
            Expression* false_expr = 0;
            
            if (!ParseExpression(state, &true_expr)) encountered_errors = true;
            else
            {
                if (!IsCurrentToken(state, Token_Colon))
                {
                    //// ERROR: missing else clause
                    encountered_errors = true;
                }
                
                else
                {
                    SkipPastToken(state, SKIP_CURRENT);
                    
                    if (!ParseExpression(state, &false_expr)) encountered_errors = true;
                    else
                    {
                        *expression = PushExpression(state, Expr_Conditional);
                        (*expression)->text                   = TextSincePos(state, condition->text.pos);
                        (*expression)->conditional.condition  = condition;
                        (*expression)->conditional.true_expr  = true_expr;
                        (*expression)->conditional.false_expr = false_expr;
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

API_FUNC bool
ParseString(Token* tokens, Memory_Arena* ast_arena, Memory_Arena* string_arena, Error_Report* error_report, AST_Node** ast)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}