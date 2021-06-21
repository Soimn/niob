// NOTE: Token kinds are organized to make parsing binary expressions easier
enum TOKEN_KIND
{
    Token_Invalid = 0,
    
    Token_Not,                                  // !
    Token_Complement,                           // ~
    Token_Increment,                            // ++
    Token_Decrement,                            // --
    Token_Period,                               // .
    Token_TripleMinus,                          // ---
    Token_Arrow,                                // ->
    Token_OpenParen,                            // (
    Token_CloseParen,                           // )
    Token_OpenBracket,                          // [
    Token_CloseBracket,                         // ]
    Token_OpenBrace,                            // {
    Token_CloseBrace,                           // }
    Token_Comma,                                // ,
    Token_Colon,                                // :
    Token_Semicolon,                            // ;
    Token_Cash,                                 // $
    Token_At,                                   // @
    Token_Underscore,                           // _
    
    Token_FirstAssignment,
    Token_Equals = Token_FirstAssignment,       // =
    Token_OrOrEquals,                           // ||=
    Token_AndAndEquals,                         // &&=
    Token_PlusEquals,                           // +=
    Token_MinusEquals,                          // -=
    Token_OrEquals,                             // |=
    Token_HatEquals,                            // ^=
    Token_StarEquals,                           // *=
    Token_SlashEquals,                          // /=
    Token_RemEquals,                            // %=
    Token_ModEquals,                            // %%=
    Token_AndEquals,                            // &=
    Token_ArithmeticRightShiftEquals,           // >>>=
    Token_RightShiftEquals,                     // >>=
    Token_LeftShiftEquals,                      // <<=
    Token_LastAssignment = Token_LeftShiftEquals,
    
    Token_FirstRangeLevel = 39,
    Token_Elipsis = Token_FirstRangeLevel,      // ..
    Token_ElipsisLess,                          // ..<
    Token_LastRangeLevel = Token_ElipsisLess,
    
    Token_FirstMulLevel = 52,
    Token_Star = Token_FirstMulLevel,           // *
    Token_Slash,                                // /
    Token_Rem,                                  // %
    Token_Mod,                                  // %%
    Token_And,                                  // &
    Token_ArithmeticRightShift,                 // >>>
    Token_RightShift,                           // >>
    Token_LeftShift,                            // <<
    Token_LastMulLevel = Token_LeftShift,
    
    Token_FirstAddLevel = 65,
    Token_Plus = Token_FirstAddLevel,           // +
    Token_Minus,                                // -
    Token_Or,                                   // |
    Token_Hat,                                  // ^
    Token_LastAddLevel = Token_Hat,
    
    Token_FirstComparative = 78,
    Token_EqualEquals = Token_FirstComparative, // ==
    Token_NotEquals,                            // !=
    Token_Less,                                 // <
    Token_Greater,                              // >
    Token_LessEquals,                           // <=
    Token_GreaterEquals,                        // >=
    Token_LastComparative = Token_GreaterEquals,
    
    Token_AndAnd = 91,                          // &&
    
    Token_OrOr = 104,                           // ||
    
    Token_Identifier,
    Token_String,
    Token_Character,
    //Token_Number // TODO: Consider BigNum
    
    Token_EndOfStream,
    
    TOKEN_COUNT
};

enum KEYWORD_KIND
{
    Keyword_Invalid = 0,
    
    Keyword_Do,
    Keyword_In,
    Keyword_Where,
    Keyword_Proc,
    Keyword_Struct,
    Keyword_Union,
    Keyword_Enum,
    Keyword_If,
    Keyword_Else,
    Keyword_When,
    Keyword_While,
    Keyword_For,
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    Keyword_True,
    Keyword_False,
    
    KEYWORD_COUNT
};

typedef struct Token
{
    u32 offset;
    u32 offset_to_line_start;
    u32 line;
    u32 size;
    
    union
    {
        struct
        {
            String string;
            Enum8(KEYWORD_KIND) keyword;
        };
        
        u32 character;
    };
    
    Enum8(TOKEN_KIND) kind;
} Token;

#define LEXER_TOKEN_BUFFER_SIZE 16
#define LEXER_TOKEN_WINDOW_SIZE 3
typedef struct Lexer
{
    String string;
    u8 at[2];
    
    u32 offset;
    u32 offset_to_line_start;
    u32 line;
    
    umm token_window_index;
    Token buffer[MAX(LEXER_TOKEN_WINDOW_SIZE, LEXER_TOKEN_BUFFER_SIZE)];
} Lexer;

// TODO: Error reporting and string allocation
void
Lexer_AdvanceCursor(Lexer* lexer)
{
    if (lexer->at[0] == '\n')
    {
        lexer->offset_to_line_start = lexer->offset + 1;
        lexer->line                += 1;
    }
    
    if (lexer->offset + 2 < lexer->string.size)
    {
        lexer->offset += 2;
        
        lexer->at[0] = lexer->string.data[lexer->offset];
        lexer->at[1] = lexer->string.data[lexer->offset + 1];
    }
    
    else if (lexer->offset + 1 < lexer->string.size)
    {
        lexer->offset += 1;
        
        lexer->at[0] = lexer->string.data[lexer->offset];
        lexer->at[1] = 0;
    }
    
    else
    {
        lexer->at[0] = 0;
        lexer->at[1] = 0;
    }
}

void
Lexer_EatTextAndFillBuffer(Lexer* lexer, umm start_index)
{
    bool encountered_errors = false;
    
    Zero(&lexer->buffer[start_index], (ARRAY_SIZE(lexer->buffer) - 2) * sizeof(Token));
    
    for (umm i = start_index; i < ARRAY_SIZE(lexer->buffer) && !encountered_errors; ++i)
    {
        while (!encountered_errors)
        {
            if (lexer->at[0] == ' '  || lexer->at[0] == '\t' ||
                lexer->at[0] == '\v' || lexer->at[0] == '\r' ||
                lexer->at[0] == '\f' || lexer->at[0] == '\n')
            {
                Lexer_AdvanceCursor(lexer);
            }
            
            else if (lexer->at[0] == '/' && (lexer->at[1] == '/' || lexer->at[1] == '*'))
            {
                if (lexer->at[1] == '/')
                {
                    while (lexer->at[0] != 0 && lexer->at[0] != '\n') Lexer_AdvanceCursor(lexer);
                    
                }
                
                else
                {
                    Lexer_AdvanceCursor(lexer);
                    Lexer_AdvanceCursor(lexer);
                    
                    umm nest_level = 1;
                    while (nest_level != 0 && lexer->at[0] != 0)
                    {
                        if (lexer->at[0] == '/' && lexer->at[1] == '*')
                        {
                            Lexer_AdvanceCursor(lexer);
                            nest_level += 1;
                        }
                        
                        else if (lexer->at[0] == '*' && lexer->at[1] == '/')
                        {
                            Lexer_AdvanceCursor(lexer);
                            nest_level -= 1;
                        }
                        
                        Lexer_AdvanceCursor(lexer);
                    }
                    
                    if (nest_level != 0)
                    {
                        //// ERROR: unterminated block comment
                        encountered_errors = true;
                    }
                }
            }
            
            else break;
        }
        
        Token* token = &lexer->buffer[i];
        token->kind                 = Token_Invalid;
        token->offset               = lexer->offset;
        token->offset_to_line_start = lexer->offset_to_line_start;
        token->line                 = lexer->line;
        
        u8 c = lexer->at[0];
        Lexer_AdvanceCursor(lexer);
        
        switch (c)
        {
            case 0: token->kind = Token_EndOfStream; break;
            
            case '~': token->kind = Token_Complement;   break;
            case '(': token->kind = Token_OpenParen;    break;
            case ')': token->kind = Token_CloseParen;   break;
            case '[': token->kind = Token_OpenBracket;  break;
            case ']': token->kind = Token_CloseBracket; break;
            case '{': token->kind = Token_OpenBrace;    break;
            case '}': token->kind = Token_CloseBrace;   break;
            case ',': token->kind = Token_Comma;        break;
            case ':': token->kind = Token_Colon;        break;
            case ';': token->kind = Token_Semicolon;    break;
            case '$': token->kind = Token_Cash;         break;
            case '@': token->kind = Token_At;           break;
            
            case '+':
            {
                token->kind = Token_Plus;
                
                if (lexer->at[0] == '+')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_Increment;
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_PlusEquals;
                }
            } break;
            
            case '-':
            {
                token->kind = Token_Minus;
                
                if (lexer->at[0] == '-')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_Decrement;
                }
                
                else if (lexer->at[0] == '-')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_TripleMinus;
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_MinusEquals;
                }
                
                else if (lexer->at[0] == '>')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_Arrow;
                }
            } break;
            
            case '*':
            {
                token->kind = Token_Star;
                
                if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_StarEquals;
                }
            } break;
            
            case '/':
            {
                token->kind = Token_Slash;
                
                if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_SlashEquals;
                }
            } break;
            
            case '^':
            {
                token->kind = Token_Hat;
                
                if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_HatEquals;
                }
            } break;
            
            case '!':
            {
                token->kind = Token_Not;
                
                if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_NotEquals;
                }
            } break;
            
            case '=':
            {
                token->kind = Token_Equals;
                
                if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_EqualEquals;
                }
            } break;
            
            case '%':
            {
                token->kind = Token_Rem;
                
                if (lexer->at[0] == '%')
                {
                    Lexer_AdvanceCursor(lexer);
                    
                    token->kind = Token_Mod;
                    
                    if (lexer->at[0] == '=')
                    {
                        Lexer_AdvanceCursor(lexer);
                        token->kind = Token_ModEquals;
                    }
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_RemEquals;
                }
            } break;
            
            case '&':
            {
                token->kind = Token_And;
                
                if (lexer->at[0] == '&')
                {
                    Lexer_AdvanceCursor(lexer);
                    
                    token->kind = Token_AndAnd;
                    
                    if (lexer->at[0] == '=')
                    {
                        Lexer_AdvanceCursor(lexer);
                        token->kind = Token_AndAndEquals;
                    }
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_AndEquals;
                }
            } break;
            
            case '|':
            {
                token->kind = Token_Or;
                
                if (lexer->at[0] == '|')
                {
                    Lexer_AdvanceCursor(lexer);
                    
                    token->kind = Token_OrOr;
                    
                    if (lexer->at[0] == '=')
                    {
                        Lexer_AdvanceCursor(lexer);
                        token->kind = Token_OrOrEquals;
                    }
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_OrOrEquals;
                }
            } break;
            
            case '<':
            {
                token->kind = Token_Less;
                
                if (lexer->at[0] == '<')
                {
                    Lexer_AdvanceCursor(lexer);
                    
                    token->kind = Token_LeftShift;
                    
                    if (lexer->at[0] == '=')
                    {
                        Lexer_AdvanceCursor(lexer);
                        token->kind = Token_LeftShiftEquals;
                    }
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_LessEquals;
                }
            } break;
            
            case '>':
            {
                token->kind = Token_Greater;
                
                if (lexer->at[0] == '>')
                {
                    Lexer_AdvanceCursor(lexer);
                    
                    token->kind = Token_RightShift;
                    
                    if (lexer->at[0] == '>')
                    {
                        Lexer_AdvanceCursor(lexer);
                        
                        token->kind = Token_ArithmeticRightShift;
                        
                        if (lexer->at[0] == '=')
                        {
                            Lexer_AdvanceCursor(lexer);
                            token->kind = Token_ArithmeticRightShiftEquals;
                        }
                    }
                    
                    else if (lexer->at[0] == '=')
                    {
                        Lexer_AdvanceCursor(lexer);
                        token->kind = Token_RightShiftEquals;
                    }
                }
                
                else if (lexer->at[0] == '=')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_GreaterEquals;
                }
            } break;
            
            case '.':
            {
                token->kind = Token_Period;
                
                if (lexer->at[0] == '.')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_Elipsis;
                }
                
                else if (lexer->at[0] == '<')
                {
                    Lexer_AdvanceCursor(lexer);
                    token->kind = Token_ElipsisLess;
                }
            } break;
            
            default:
            {
                if (c == '_'             ||
                    c >= 'a' && c <= 'z' ||
                    c >= 'A' && c <= 'Z')
                {
                    if (!(c == '_'             ||
                          c >= 'a' && c <= 'z' ||
                          c >= 'A' && c <= 'Z'))
                    {
                        token->kind = Token_Underscore;
                    }
                    
                    else
                    {
                        token->kind = Token_Identifier;
                        
                        token->string.data = &lexer->string.data[lexer->offset - 1];
                        token->string.size = 1;
                        
                        while (lexer->at[0] == '_'                        ||
                               lexer->at[0] >= 'a' && lexer->at[0] <= 'z' ||
                               lexer->at[0] >= 'A' && lexer->at[0] <= 'Z' ||
                               lexer->at[0] >= '0' && lexer->at[0] <= '9')
                        {
                            Lexer_AdvanceCursor(lexer);
                        }
                        
                        NOT_IMPLEMENTED; // keywords
                    }
                }
                
                else if (c >= '0' && c <= '9')
                {
                    NOT_IMPLEMENTED;
                }
                
                else if (c == '\'' || c == '"')
                {
                    NOT_IMPLEMENTED;
                }
                
                else
                {
                    //// ERROR: Unknown symbol
                    encountered_errors = true;
                }
            } break;
        }
        
        token->size = lexer->offset - token->offset;
    }
}

Lexer
Lexer_Init(String string)
{
    Lexer lexer = {0};
    lexer.string = string;
    lexer.line   = 1;
    
    Lexer_EatTextAndFillBuffer(&lexer, 0);
    
    return lexer;
}

void
Lexer_AdvanceTokenWindow(Lexer* lexer)
{
    lexer->token_window_index += 1;
    
    if (lexer->token_window_index > ARRAY_SIZE(lexer->buffer) - LEXER_TOKEN_WINDOW_SIZE)
    {
        Copy(&lexer->buffer[ARRAY_SIZE(lexer->buffer) - 2], lexer->buffer, sizeof(Token) * 2);
        lexer->token_window_index = 0;
        
        Lexer_EatTextAndFillBuffer(lexer, 2);
    }
}