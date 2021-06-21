typedef struct Parser_State
{
    Lexer* lexer;
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

Token
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
ParsePostfixExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

bool
ParsePrefixExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_Plus) SkipPastCurrentToken(state);
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
            if (!ParsePrefixExpression(state, operand))
            {
                encountered_errors = true;
            }
        }
    }
    
    else
    {
        Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
        
        switch (token.kind)
        {
            case Token_Minus:      kind = Expr_Negation;        break;
            case Token_Complement: kind = Expr_Complement;      break;
            case Token_Increment:  kind = Expr_PreIncrement;    break;
            case Token_Decrement:  kind = Expr_PreDecrement;    break;
            case Token_And:        kind = Expr_Reference;       break;
            case Token_Star:       kind = Expr_Dereference;     break;
            case Token_Hat:        kind = Expr_PointerType;     break;
            case Token_Cash:       kind = Expr_PolymorphicType; break;
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
            
            Enum8(EXPRESSION_KIND) op = (token.kind == Token_Identifier ? Expr_InfixCall : token.kind);
            umm precedence            = op / 13;
            
            // precedence < 3 || precedence > 8
            if (precedence - 3 > 5) break;
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
ParseText(String string)
{
    Lexer lexer = Lexer_Init(string);
    
    Parser_State state = {0};
    state.lexer = &lexer;
}