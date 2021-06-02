typedef struct Parser_State
{
    Memory_Arena* ast_arena;
    Memory_Arena* string_arena;
    
    u32 offset_to_end_of_last_token;
    Token* current_token;
} Parser_State;

void
SkipCurrentToken(Parser_State state)
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

Token
CurrentToken(Parser_State state)
{
    return *state.current_token;
}

Token
PeekToken(Parser_State state)
{
    return *state.current_token;
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

Expression*
PushExpression(Parser_State state, Enum8(EXPRESSION_KIND) kind)
{
    Expression* result = Arena_PushSize(state.ast_arena, sizeof(Expression), ALIGNOF(Expression));
    ZeroStruct(result);
    
    result->kind = kind;
    
    return result;
}

Statement*
PushStatement(Parser_State state, Enum8(STATEMENT_KIND) kind)
{
    Statement* result = Arena_PushSize(state.ast_arena, sizeof(Statement), ALIGNOF(Statement));
    ZeroStruct(result);
    
    result->kind = kind;
    
    return result;
}

bool ParseExpression(Parser_State state, Expression** expression);

bool
ParseNamedArgumentList(Parser_State state, Named_Argument* arguments)
{
    bool encountered_errors = false;
    
    Named_Argument* last_argument = 0;
    while (!encountered_errors)
    {
        Named_Argument* argument = 0;
        
        if (last_argument == 0)
        {
            argument      = arguments;
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
                SkipCurrentToken(state);
                
                argument->name = argument->value;
                if (!ParseExpression(state, &argument->value)) encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                if (IsCurrentToken(state, Token_Comma)) SkipCurrentToken(state);
                else break;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParsePrimaryExpression(Parser_State state, Expression** expression)
{
    bool encountered_errors = false;
    
    Text_Pos start_text_pos = CurrentTextPos(state);
    
    if (IsCurrentToken(state, Token_OpenParen))
    {
        SkipCurrentToken(state);
        
        *expression = PushExpression(state, Expr_Compound);
        
        if (!ParseExpression(state, &(*expression)->compound_expression)) encountered_errors = true;
        else
        {
            if (!IsCurrentToken(state, Token_CloseParen))
            {
                //// ERROR: Missing matching closing paren
                encountered_errors = true;
            }
            
            else
            {
                SkipCurrentToken(state);
                
                (*expression)->text = TextSincePos(state, start_text_pos);
            }
        }
    }
    
    else if (IsCurrentToken(state, Token_String))
    {
        Token token = CurrentToken(state);
        
        *expression = PushExpression(state, Expr_String);
        (*expression)->string = token.string;
        (*expression)->text   = token.text;
        
        SkipCurrentToken(state);
    }
    
    else if (IsCurrentToken(state, Token_Character))
    {
        Token token = CurrentToken(state);
        
        *expression = PushExpression(state, Expr_Char);
        (*expression)->character = token.character;
        (*expression)->text      = token.text;
        
        SkipCurrentToken(state);
    }
    
    else if (IsCurrentToken(state, Token_Int) ||
             IsCurrentToken(state, Token_Float))
    {
        // TODO: precision handling and evaluating wether it can be represented as an int
        NOT_IMPLEMENTED;
    }
    
    else if (IsCurrentToken(state, Token_Identifier))
    {
        Token token = CurrentToken(state);
        
        if (token.keyword == Keyword_Proc)
        {
            NOT_IMPLEMENTED;
        }
        
        else if (token.keyword == Keyword_Struct)
        {
            NOT_IMPLEMENTED;
        }
        
        else if (token.keyword == Keyword_Union)
        {
            NOT_IMPLEMENTED;
        }
        
        else if (token.keyword == Keyword_Enum)
        {
            NOT_IMPLEMENTED;
        }
        
        else if (token.keyword == Keyword_True ||
                 token.keyword == Keyword_False)
        {
            *expression = PushExpression(state, Expr_Boolean);
            (*expression)->boolean = (token.keyword == Keyword_True);
            (*expression)->text    = token.text;
            
            SkipCurrentToken(state);
        }
        
        else if (token.keyword == Keyword_Invalid)
        {
            *expression = PushExpression(state, Expr_Identifier);
            (*expression)->identifier = token.string;
            (*expression)->text       = token.text;
            
            SkipCurrentToken(state);
        }
        
        else
        {
            //// ERROR: Illegal use of keyword in expression
            encountered_errors = true;
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
                SkipCurrentToken(state);
                
                Expression* pointer = *expression;
                Expression* index   = 0;
                
                if (!ParseExpression(state, &index)) encountered_errors = true;
                else
                {
                    if (IsCurrentToken(state, Token_Colon))
                    {
                        SkipCurrentToken(state);
                        
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
                        SkipCurrentToken(state);
                        (*expression)->text = TextSincePos(state, (*expression)->slice.pointer->text.pos);
                    }
                }
            }
            
            else if (IsCurrentToken(state, Token_OpenParen))
            {
                SkipCurrentToken(state);
                
                Expression* pointer = *expression;
                
                *expression = PushExpression(state, Expr_Call);
                (*expression)->call.pointer = pointer;
                
                if (!ParseNamedArgumentList(state, &(*expression)->call.arguments)) encountered_errors = true;
                else
                {
                    if (!IsCurrentToken(state, Token_CloseParen))
                    {
                        //// ERROR: Missing closing parenthesis
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        SkipCurrentToken(state);
                        
                        (*expression)->text = TextSincePos(state, (*expression)->call.pointer->text.pos);
                    }
                }
            }
            
            else if (IsCurrentToken(state, Token_Period))
            {
                if (IsCurrentToken(state, Token_OpenBrace) ||
                    IsCurrentToken(state, Token_OpenBracket))
                {
                    Enum32(TOKEN_KIND) token = CurrentTokenKind(state);
                    SkipCurrentToken(state);
                    
                    Expression* type          = *expression;
                    Named_Argument* arguments = 0;
                    
                    if (token == Token_OpenBrace)
                    {
                        *expression = PushExpression(state, Expr_StructLiteral);
                        (*expression)->struct_literal.type = type;
                        
                        arguments = &(*expression)->struct_literal.arguments;
                    }
                    
                    else
                    {
                        *expression = PushExpression(state, Expr_ArrayLiteral);
                        (*expression)->array_literal.type = type;
                        
                        arguments = &(*expression)->array_literal.arguments;
                    }
                    
                    if (!ParseNamedArgumentList(state, arguments)) encountered_errors = true;
                    else
                    {
                        if (token == Token_OpenBrace && !IsCurrentToken(state, Token_CloseBrace))
                        {
                            //// ERROR: Missing matching closing brace after struct literal
                            encountered_errors = true;
                        }
                        
                        else if (!IsCurrentToken(state, Token_CloseBracket))
                        {
                            //// ERROR: Missing matching closing bracket after array literal
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            SkipCurrentToken(state);
                            
                            (*expression)->text = TextSincePos(state, type->text.pos);
                        }
                    }
                }
                
                else
                {
                    Expression* left = *expression;
                    
                    *expression = PushExpression(state, Expr_Member);
                    (*expression)->left = left;
                    
                    if (!ParsePrimaryExpression(state, &(*expression)->right)) encountered_errors = true;
                    else (*expression)->text = TextSincePos(state, left->text.pos);
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
    
    Text_Pos start_text_pos = CurrentTextPos(state);
    
    if (IsCurrentToken(state, Token_Period))
    {
        SkipCurrentToken(state);
        
        if (IsCurrentToken(state, Token_OpenBrace) ||
            IsCurrentToken(state, Token_OpenBracket))
        {
            Enum32(TOKEN_KIND) token = CurrentTokenKind(state);
            SkipCurrentToken(state);
            
            Named_Argument* arguments = 0;
            
            if (token == Token_OpenBrace)
            {
                *expression = PushExpression(state, Expr_StructLiteral);
                (*expression)->struct_literal.type = 0;
                
                arguments = &(*expression)->struct_literal.arguments;
            }
            
            else
            {
                *expression = PushExpression(state, Expr_ArrayLiteral);
                (*expression)->array_literal.type = 0;
                
                arguments = &(*expression)->array_literal.arguments;
            }
            
            if (!ParseNamedArgumentList(state, arguments)) encountered_errors = true;
            else
            {
                if (token == Token_OpenBrace && !IsCurrentToken(state, Token_CloseBrace))
                {
                    //// ERROR: Missing matching closing brace after struct literal
                    encountered_errors = true;
                }
                
                else if (!IsCurrentToken(state, Token_CloseBracket))
                {
                    //// ERROR: Missing matching closing bracket after array literal
                    encountered_errors = true;
                }
                
                else
                {
                    SkipCurrentToken(state);
                    
                    (*expression)->text = TextSincePos(state, start_text_pos);
                }
            }
        }
        
        else
        {
            *expression = PushExpression(state, Expr_Member);
            (*expression)->left = 0;
            
            if (!ParsePrimaryExpression(state, &(*expression)->right)) encountered_errors = true;
            else
            {
                (*expression)->text = TextSincePos(state, start_text_pos);
            }
        }
    }
    
    else if (CurrentTokenKind(state) == Token_OpenBracket)
    {
        SkipCurrentToken(state);
        
        Expression** operand = 0;
        if (CurrentTokenKind(state) == Token_CloseBracket)
        {
            SkipCurrentToken(state);
            
            *expression = PushExpression(state, Expr_SliceType);
            
            operand = &(*expression)->operand;
        }
        
        else
        {
            if (CurrentTokenKind(state) == Token_PeriodPeriod)
            {
                SkipCurrentToken(state);
                
                *expression = PushExpression(state, Expr_DynamicArrayType);
                
                operand = &(*expression)->operand;
            }
            
            else
            {
                *expression = PushExpression(state, Expr_ArrayType);
                
                if (!ParseExpression(state, &(*expression)->array_type.size)) encountered_errors = true;
                else
                {
                    operand = &(*expression)->array_type.elem_type;
                }
            }
            
            if (!encountered_errors)
            {
                if (CurrentTokenKind(state) == Token_CloseBracket) SkipCurrentToken(state);
                else
                {
                    //// ERROR: Missing closing bracket
                    encountered_errors = true;
                }
            }
        }
        
        if (!encountered_errors)
        {
            if (!ParsePrefixExpression(state, operand)) encountered_errors = true;
            else
            {
                (*expression)->text = TextSincePos(state, start_text_pos);
            }
        }
    }
    
    else
    {
        Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
        
        switch (CurrentTokenKind(state))
        {
            case Token_Plus:   kind = Expr_Plus;        break;
            case Token_Minus:  kind = Expr_Minus;       break;
            case Token_BitAnd: kind = Expr_Reference;   break;
            case Token_Star:   kind = Expr_Dereference; break;
            case Token_BitNot: kind = Expr_BitNot;      break;
            case Token_Not:    kind = Expr_Not;         break;
            case Token_Hat:    kind = Expr_PointerType; break;
        }
        
        if (kind == Expr_Invalid) encountered_errors = !ParsePostfixExpression(state, expression);
        else
        {
            SkipCurrentToken(state);
            
            *expression = PushExpression(state, kind);
            
            if (!ParsePrefixExpression(state, &(*expression)->operand)) encountered_errors = true;
            else
            {
                (*expression)->text = TextSincePos(state, start_text_pos);
            }
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
                SkipCurrentToken(state);
                
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
            SkipCurrentToken(state);
            
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
                    SkipCurrentToken(state);
                    
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

bool
ParseStatement(Parser_State state, Statement** statement)
{
    bool encountered_errors = false;
    
    Text_Pos start_of_statement = CurrentTextPos(state);
    
    if (IsCurrentToken(state, Token_EndOfStream))
    {
        //// ERROR: Encountered end of stream
        encountered_errors = true;
    }
    
    else if (IsCurrentToken(state, Token_EndOfStream))
    {
        //// ERROR: Stray semicolon
        encountered_errors = true;
    }
    
    else if (IsCurrentToken(state, Token_OpenBrace))
    {
        NOT_IMPLEMENTED;
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && (CurrentToken(state).keyword == Keyword_If ||
                                                         CurrentToken(state).keyword == Keyword_When))
    {
        bool is_if = (CurrentToken(state).keyword == Keyword_If);
        
        SkipCurrentToken(state);
        
        NOT_IMPLEMENTED;
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Else)
    {
        //// ERROR: Illegal else wihout matching if/when
        encountered_errors = true;
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && (CurrentToken(state).keyword == Keyword_Break ||
                                                         CurrentToken(state).keyword == Keyword_Continue))
    {
        bool is_break = (CurrentToken(state).keyword == Keyword_Break);
        
        SkipCurrentToken(state);
        
        if (is_break) *statement = PushStatement(state, Statement_Break);
        else          *statement = PushStatement(state, Statement_Continue);
        
        if (!IsCurrentToken(state, Token_Semicolon))
        {
            if (!ParseExpression(state, (is_break ? &(*statement)->break_statement.label : &(*statement)->continue_statement.label)))
            {
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors)
        {
            if (!IsCurrentToken(state, Token_Semicolon))
            {
                //// ERROR: Missing terminating semicolon after continue/break
                encountered_errors = true;
            }
            
            else
            {
                SkipCurrentToken(state);
                
                (*statement)->text = TextSincePos(state, start_of_statement);
            }
        }
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Defer)
    {
        SkipCurrentToken(state);
        
        *statement = PushStatement(state, Statement_Defer);
        
        if (!ParseStatement(state, &(*statement)->defer_statement.statement)) encountered_errors = true;
        else
        {
            (*statement)->text = TextSincePos(state, start_of_statement);
        }
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Return)
    {
        SkipCurrentToken(state);
        
        *statement = PushStatement(state, Statement_Return);
        
        if (!IsCurrentToken(state, Token_Semicolon))
        {
            if (!ParseNamedArgumentList(state, &(*statement)->return_statement.arguments))
            {
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors)
        {
            if (!IsCurrentToken(state, Token_Semicolon))
            {
                //// ERROR: Missing terminating semicolon after return statement
                encountered_errors = true;
            }
            
            else
            {
                SkipCurrentToken(state);
                
                (*statement)->text = TextSincePos(state, start_of_statement);
            }
        }
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Import ||
             (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Using &&
              IsPeekToken(state, Token_Identifier) && PeekToken(state).keyword == Keyword_Import))
    {
        NOT_IMPLEMENTED;
    }
    
    else if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Include ||
             (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Using &&
              IsPeekToken(state, Token_Identifier) && PeekToken(state).keyword == Keyword_Include))
    {
        NOT_IMPLEMENTED;
    }
    
    else
    {
        bool is_using           = false;
        Expression* expressions = 0;
        
        {
            if (IsCurrentToken(state, Token_Identifier) && CurrentToken(state).keyword == Keyword_Using)
            {
                is_using = true;
                
                SkipCurrentToken(state);
            }
            
            Expression** current_expression = &expressions;
            
            while (!encountered_errors)
            {
                if (!ParseExpression(state, current_expression)) encountered_errors = true;
                else
                {
                    if (IsCurrentToken(state, Token_Comma))
                    {
                        SkipCurrentToken(state);
                        
                        current_expression = &(*current_expression)->next;
                    }
                    
                    else break;
                }
            }
        }
        
        if (!encountered_errors)
        {
            if (IsCurrentToken(state, Token_Colon))
            {
                SkipCurrentToken(state);
                
                if (IsCurrentToken(state, Token_OpenBrace) ||
                    IsCurrentToken(state, Token_Identifier) && (CurrentToken(state).keyword == Keyword_If   ||
                                                                CurrentToken(state).keyword == Keyword_When ||
                                                                CurrentToken(state).keyword == Keyword_Else))
                {
                    if (expressions->next != 0)
                    {
                        //// ERROR: multiple labels
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        if (is_using)
                        {
                            //// ERROR: Illegal use of using on labeled statement
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            if (!ParseStatement(state, statement)) encountered_errors = true;
                            else
                            {
                                if      ((*statement)->kind == Statement_Block) (*statement)->block.label          = expressions;
                                else if ((*statement)->kind == Statement_If)    (*statement)->if_statement.label   = expressions;
                                else                                            (*statement)->when_statement.label = expressions;
                                
                                (*statement)->text = TextSincePos(state, expressions->text.pos);
                            }
                        }
                    }
                }
                
                else
                {
                    Expression* names  = expressions;
                    Expression* type   = 0;
                    Expression* values = 0;
                    bool is_uninitialized = false;
                    bool is_constant      = false;
                    
                    if (!IsCurrentToken(state, Token_Equal))
                    {
                        if (!ParseExpression(state, &type)) encountered_errors = true;
                        else
                        {
                            if (IsCurrentToken(state, Token_Comma))
                            {
                                //// ERROR: Declarations support only one type specifier
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    if (!encountered_errors && (IsCurrentToken(state, Token_Equal) ||
                                                IsCurrentToken(state, Token_Colon)))
                    {
                        is_constant = IsCurrentToken(state, Token_Colon);
                        
                        SkipCurrentToken(state);
                        
                        if (IsCurrentToken(state, Token_MinusMinusMinus))
                        {
                            if (is_constant)
                            {
                                //// ERROR: Constants cannot be left uninitialized
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                is_uninitialized = true;
                                
                                SkipCurrentToken(state);
                            }
                        }
                        
                        else
                        {
                            Expression** current_expression = &values;
                            
                            while (!encountered_errors)
                            {
                                if (!ParseExpression(state, current_expression)) encountered_errors = true;
                                else
                                {
                                    if (IsCurrentToken(state, Token_Comma))
                                    {
                                        SkipCurrentToken(state);
                                        
                                        current_expression = &(*current_expression)->next;
                                    }
                                    
                                    else break;
                                }
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (is_constant)
                        {
                            *statement = PushStatement(state, Statement_ConstDecl);
                            (*statement)->constant_decl.names    = names;
                            (*statement)->constant_decl.type     = type;
                            (*statement)->constant_decl.values   = values;
                            (*statement)->constant_decl.is_using = is_using;
                        }
                        
                        else
                        {
                            *statement = PushStatement(state, Statement_VarDecl);
                            (*statement)->variable_decl.names            = names;
                            (*statement)->variable_decl.type             = type;
                            (*statement)->variable_decl.values           = values;
                            (*statement)->variable_decl.is_uninitialized = is_uninitialized;
                            (*statement)->variable_decl.is_using         = is_using;
                        }
                        
                        if (!IsCurrentToken(state, Token_Semicolon))
                        {
                            //// ERROR: Missing terminating semicolon after declaration
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            SkipCurrentToken(state);
                            
                            (*statement)->text = TextSincePos(state, expressions->text.pos);
                        }
                    }
                }
            }
            
            else
            {
                Enum8(EXPRESSION_KIND) kind = Expr_Invalid;
                
                switch (CurrentTokenKind(state))
                {
                    case Token_OrEqual:             kind = Expr_Or;          break;
                    case Token_AndEqual:            kind = Expr_And;         break;
                    case Token_PlusEqual:           kind = Expr_Add;         break;
                    case Token_MinusEqual:          kind = Expr_Sub;         break;
                    case Token_BitOrEqual:          kind = Expr_BitOr;       break;
                    case Token_HatEqual:            kind = Expr_BitXor;      break;
                    case Token_StarEqual:           kind = Expr_Mul;         break;
                    case Token_SlashEqual:          kind = Expr_Div;         break;
                    case Token_PercentageEqual:     kind = Expr_Rem;         break;
                    case Token_BitAndEqual:         kind = Expr_BitAnd;      break;
                    case Token_LessLessEqual:       kind = Expr_LShift;      break;
                    case Token_GreaterGreaterEqual: kind = Expr_RShift;      break;
                }
                
                if (IsCurrentToken(state, Token_Equal) || kind != Expr_Invalid)
                {
                    SkipCurrentToken(state);
                    
                    if (is_using)
                    {
                        //// ERROR: Illegal use of using on assignment statement
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        *statement = PushStatement(state, Statement_Assignment);
                        (*statement)->assignment_statement.op  = kind;
                        (*statement)->assignment_statement.lhs = expressions;
                        
                        Expression** current_expression = &(*statement)->assignment_statement.rhs;
                        
                        while (!encountered_errors)
                        {
                            if (!ParseExpression(state, current_expression)) encountered_errors = true;
                            else
                            {
                                if (IsCurrentToken(state, Token_Comma))
                                {
                                    SkipCurrentToken(state);
                                    
                                    current_expression = &(*current_expression)->next;
                                }
                                
                                else break;
                            }
                        }
                    }
                }
                
                else
                {
                    if (expressions->next != 0)
                    {
                        //// ERROR: Use of comma separated expressions is illegal outside assignment statements and argument lists
                        encountered_errors = true;
                    }
                    
                    else if (!IsCurrentToken(state, Token_Semicolon))
                    {
                        //// ERROR: Missing semicolon after expression statement
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        SkipCurrentToken(state);
                        
                        if (is_using)
                        {
                            *statement = PushStatement(state, Statement_Using);
                            (*statement)->using_statement.symbol = expressions;
                            (*statement)->text                   = TextSincePos(state, expressions->text.pos);
                        }
                        
                        else
                        {
                            *statement = PushStatement(state, Statement_Expression);
                            (*statement)->expression = expressions;
                            (*statement)->text       = TextSincePos(state, expressions->text.pos);
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

API_FUNC bool
ParseString(Token* tokens, Memory_Arena* ast_arena, Memory_Arena* string_arena, Error_Report* error_report, Statement** ast)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}