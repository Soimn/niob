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
    Token_QuestionMark,                         // ?
    Token_Pound,                                // #
    
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
    
    Token_FirstRangeLevel = 80,
    Token_Elipsis = Token_FirstRangeLevel,      // ..
    Token_ElipsisLess,                          // ..<
    Token_LastRangeLevel = Token_ElipsisLess,
    
    Token_FirstMulLevel = 100,
    Token_Star = Token_FirstMulLevel,           // *
    Token_Slash,                                // /
    Token_Rem,                                  // %
    Token_Mod,                                  // %%
    Token_And,                                  // &
    Token_ArithmeticRightShift,                 // >>>
    Token_RightShift,                           // >>
    Token_LeftShift,                            // <<
    Token_LastMulLevel = Token_LeftShift,
    
    Token_FirstAddLevel = 120,
    Token_Plus = Token_FirstAddLevel,           // +
    Token_Minus,                                // -
    Token_Or,                                   // |
    Token_Hat,                                  // ^
    Token_LastAddLevel = Token_Hat,
    
    Token_FirstComparative = 140,
    Token_EqualEquals = Token_FirstComparative, // ==
    Token_NotEquals,                            // !=
    Token_Less,                                 // <
    Token_Greater,                              // >
    Token_LessEquals,                           // <=
    Token_GreaterEquals,                        // >=
    Token_LastComparative = Token_GreaterEquals,
    
    Token_AndAnd = 160,                         // &&
    
    Token_OrOr = 180,                           // ||
    
    Token_Identifier,
    Token_String,
    Token_Character,
    Token_Int,
    Token_Float,
    
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
    Keyword_True,
    Keyword_False,
    Keyword_As,
    
    Keyword_FirstStatementInitiator,
    Keyword_If = Keyword_FirstStatementInitiator,
    Keyword_Else,
    Keyword_When,
    Keyword_While,
    Keyword_For,
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    Keyword_Import,
    Keyword_Foreign,
    Keyword_Unreachable,
    Keyword_NotImplemented,
    Keyword_LastStatementInitiator = Keyword_NotImplemented,
    
    KEYWORD_COUNT
};

typedef struct Token
{
    u32 offset;
    u32 offset_to_line_start;
    u32 line;
    u16 size;
    Enum8(TOKEN_KIND) kind;
    
    union
    {
        struct
        {
            Identifier identifier;
            Enum8(KEYWORD_KIND) keyword;
        };
        
        String string;
        
        u32 character;
        
        u64 integer;
        
        f64 floating;
    };
} Token;

typedef struct Lexer
{
    String string;
    u8 at[2];
    
    u32 offset;
    u32 offset_to_line_start;
    u32 line;
    
    Identifier keywords[KEYWORD_COUNT];
} Lexer;

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
Lexer_ReportError(Lexer* lexer, ...)
{
    NOT_IMPLEMENTED;
}

bool
LexText(Workspace* workspace, String text, Memory_Arena* token_arena, Memory_Arena* string_arena, Bucket_Array(Token)* token_array)
{
    Lexer lexer = {0};
    lexer.string    = text;
    lexer.line      = 1;
    
    lexer.keywords[Keyword_Invalid]        = 0;
    lexer.keywords[Keyword_Do]             = WS_GetIdentifier(workspace, STR("do"));
    lexer.keywords[Keyword_In]             = WS_GetIdentifier(workspace, STR("in"));
    lexer.keywords[Keyword_Where]          = WS_GetIdentifier(workspace, STR("where"));
    lexer.keywords[Keyword_Proc]           = WS_GetIdentifier(workspace, STR("proc"));
    lexer.keywords[Keyword_Struct]         = WS_GetIdentifier(workspace, STR("struct"));
    lexer.keywords[Keyword_Union]          = WS_GetIdentifier(workspace, STR("union"));
    lexer.keywords[Keyword_Enum]           = WS_GetIdentifier(workspace, STR("enum"));
    lexer.keywords[Keyword_If]             = WS_GetIdentifier(workspace, STR("if"));
    lexer.keywords[Keyword_Else]           = WS_GetIdentifier(workspace, STR("else"));
    lexer.keywords[Keyword_When]           = WS_GetIdentifier(workspace, STR("when"));
    lexer.keywords[Keyword_While]          = WS_GetIdentifier(workspace, STR("while"));
    lexer.keywords[Keyword_For]            = WS_GetIdentifier(workspace, STR("for"));
    lexer.keywords[Keyword_Break]          = WS_GetIdentifier(workspace, STR("break"));
    lexer.keywords[Keyword_Continue]       = WS_GetIdentifier(workspace, STR("continue"));
    lexer.keywords[Keyword_Using]          = WS_GetIdentifier(workspace, STR("using"));
    lexer.keywords[Keyword_Defer]          = WS_GetIdentifier(workspace, STR("defer"));
    lexer.keywords[Keyword_Return]         = WS_GetIdentifier(workspace, STR("return"));
    lexer.keywords[Keyword_True]           = WS_GetIdentifier(workspace, STR("true"));
    lexer.keywords[Keyword_False]          = WS_GetIdentifier(workspace, STR("false"));
    lexer.keywords[Keyword_Import]         = WS_GetIdentifier(workspace, STR("import"));
    lexer.keywords[Keyword_Foreign]        = WS_GetIdentifier(workspace, STR("foreign"));
    lexer.keywords[Keyword_As]             = WS_GetIdentifier(workspace, STR("as"));
    lexer.keywords[Keyword_Unreachable]    = WS_GetIdentifier(workspace, STR("unreachable"));
    lexer.keywords[Keyword_NotImplemented] = WS_GetIdentifier(workspace, STR("not_implemented"));
    
    bool encountered_errors = false;
    
    while (!encountered_errors)
    {
        while (!encountered_errors)
        {
            if (lexer.at[0] == ' '  || lexer.at[0] == '\t' ||
                lexer.at[0] == '\v' || lexer.at[0] == '\r' ||
                lexer.at[0] == '\f' || lexer.at[0] == '\n')
            {
                Lexer_AdvanceCursor(&lexer);
            }
            
            else if (lexer.at[0] == '/' && (lexer.at[1] == '/' || lexer.at[1] == '*'))
            {
                if (lexer.at[1] == '/')
                {
                    while (lexer.at[0] != 0 && lexer.at[0] != '\n') Lexer_AdvanceCursor(&lexer);
                    
                }
                
                else
                {
                    Lexer_AdvanceCursor(&lexer);
                    Lexer_AdvanceCursor(&lexer);
                    
                    umm nest_level = 1;
                    while (nest_level != 0 && lexer.at[0] != 0)
                    {
                        if (lexer.at[0] == '/' && lexer.at[1] == '*')
                        {
                            Lexer_AdvanceCursor(&lexer);
                            nest_level += 1;
                        }
                        
                        else if (lexer.at[0] == '*' && lexer.at[1] == '/')
                        {
                            Lexer_AdvanceCursor(&lexer);
                            nest_level -= 1;
                        }
                        
                        Lexer_AdvanceCursor(&lexer);
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
        
        Token* token = BA_Push(token_array);
        token->kind                 = Token_Invalid;
        token->offset               = lexer.offset;
        token->offset_to_line_start = lexer.offset_to_line_start;
        token->line                 = lexer.line;
        
        u8 c = lexer.at[0];
        Lexer_AdvanceCursor(&lexer);
        
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
            case '?': token->kind = Token_QuestionMark; break;
            case '#': token->kind = Token_Pound;        break;
            
            case '+':
            {
                token->kind = Token_Plus;
                
                if (lexer.at[0] == '+')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_Increment;
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_PlusEquals;
                }
            } break;
            
            case '-':
            {
                token->kind = Token_Minus;
                
                if (lexer.at[0] == '-')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_Decrement;
                }
                
                else if (lexer.at[0] == '-')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_TripleMinus;
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_MinusEquals;
                }
                
                else if (lexer.at[0] == '>')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_Arrow;
                }
            } break;
            
            case '*':
            {
                token->kind = Token_Star;
                
                if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_StarEquals;
                }
            } break;
            
            case '/':
            {
                token->kind = Token_Slash;
                
                if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_SlashEquals;
                }
            } break;
            
            case '^':
            {
                token->kind = Token_Hat;
                
                if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_HatEquals;
                }
            } break;
            
            case '!':
            {
                token->kind = Token_Not;
                
                if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_NotEquals;
                }
            } break;
            
            case '=':
            {
                token->kind = Token_Equals;
                
                if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_EqualEquals;
                }
            } break;
            
            case '%':
            {
                token->kind = Token_Rem;
                
                if (lexer.at[0] == '%')
                {
                    Lexer_AdvanceCursor(&lexer);
                    
                    token->kind = Token_Mod;
                    
                    if (lexer.at[0] == '=')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        token->kind = Token_ModEquals;
                    }
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_RemEquals;
                }
            } break;
            
            case '&':
            {
                token->kind = Token_And;
                
                if (lexer.at[0] == '&')
                {
                    Lexer_AdvanceCursor(&lexer);
                    
                    token->kind = Token_AndAnd;
                    
                    if (lexer.at[0] == '=')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        token->kind = Token_AndAndEquals;
                    }
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_AndEquals;
                }
            } break;
            
            case '|':
            {
                token->kind = Token_Or;
                
                if (lexer.at[0] == '|')
                {
                    Lexer_AdvanceCursor(&lexer);
                    
                    token->kind = Token_OrOr;
                    
                    if (lexer.at[0] == '=')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        token->kind = Token_OrOrEquals;
                    }
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_OrOrEquals;
                }
            } break;
            
            case '<':
            {
                token->kind = Token_Less;
                
                if (lexer.at[0] == '<')
                {
                    Lexer_AdvanceCursor(&lexer);
                    
                    token->kind = Token_LeftShift;
                    
                    if (lexer.at[0] == '=')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        token->kind = Token_LeftShiftEquals;
                    }
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_LessEquals;
                }
            } break;
            
            case '>':
            {
                token->kind = Token_Greater;
                
                if (lexer.at[0] == '>')
                {
                    Lexer_AdvanceCursor(&lexer);
                    
                    token->kind = Token_RightShift;
                    
                    if (lexer.at[0] == '>')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        
                        token->kind = Token_ArithmeticRightShift;
                        
                        if (lexer.at[0] == '=')
                        {
                            Lexer_AdvanceCursor(&lexer);
                            token->kind = Token_ArithmeticRightShiftEquals;
                        }
                    }
                    
                    else if (lexer.at[0] == '=')
                    {
                        Lexer_AdvanceCursor(&lexer);
                        token->kind = Token_RightShiftEquals;
                    }
                }
                
                else if (lexer.at[0] == '=')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_GreaterEquals;
                }
            } break;
            
            case '.':
            {
                token->kind = Token_Period;
                
                if (lexer.at[0] == '.')
                {
                    Lexer_AdvanceCursor(&lexer);
                    token->kind = Token_Elipsis;
                }
                
                else if (lexer.at[0] == '<')
                {
                    Lexer_AdvanceCursor(&lexer);
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
                        
                        String ident = {
                            .data = &lexer.string.data[lexer.offset - 1],
                            .size = 1
                        };
                        
                        while (lexer.at[0] == '_'                        ||
                               lexer.at[0] >= 'a' && lexer.at[0] <= 'z' ||
                               lexer.at[0] >= 'A' && lexer.at[0] <= 'Z' ||
                               lexer.at[0] >= '0' && lexer.at[0] <= '9')
                        {
                            Lexer_AdvanceCursor(&lexer);
                            ++ident.size;
                        }
                        
                        token->identifier = WS_GetIdentifier(workspace, ident);
                        
                        token->keyword = Keyword_Invalid;
                        for (umm j = 1; j < KEYWORD_COUNT; ++j)
                        {
                            if (token->identifier == lexer.keywords[j])
                            {
                                token->keyword = (u8)j;
                                break;
                            }
                        }
                    }
                }
                
                else if (c >= '0' && c <= '9')
                {
                    NOT_IMPLEMENTED;
                }
                
                else if (c == '\'' || c == '"')
                {
                    String raw_string = {
                        .data = &lexer.string.data[lexer.offset],
                        .size = 0,
                    };
                    
                    while (lexer.at[0] != 0 && lexer.at[0] != c)
                    {
                        if (lexer.at[0] == '\\') Lexer_AdvanceCursor(&lexer);
                        Lexer_AdvanceCursor(&lexer);
                    }
                    
                    if (lexer.at[0] == 0)
                    {
                        //// ERROR: Unterminated string/character literal
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        raw_string.size = &lexer.string.data[lexer.offset] - raw_string.data;
                        Lexer_AdvanceCursor(&lexer);
                        
                        String string = {
                            .data = Arena_PushSize(string_arena, raw_string.size, 1),
                            .size = 0
                        };
                        
                        for (umm j = 0; j < raw_string.size && !encountered_errors; ++j)
                        {
                            if (raw_string.data[j] == '\\')
                            {
                                j += 1;
                                
                                if (raw_string.data[j] == 'u' || raw_string.data[j] == 'x')
                                {
                                    umm req_digit_count = (raw_string.data[j] == 'u' ? 6 : 2);
                                    
                                    umm codepoint = 0;
                                    
                                    for (umm digit_count = 0; digit_count < req_digit_count; ++digit_count, ++j)
                                    {
                                        if (raw_string.data[j + 1] >= '0' && raw_string.data[j + 1] <= '9')
                                        {
                                            codepoint *= 16;
                                            codepoint += raw_string.data[j + 1] - '0';
                                        }
                                        
                                        else if (raw_string.data[j + 1] >= 'a' && raw_string.data[j + 1] <= 'f')
                                        {
                                            codepoint *= 16;
                                            codepoint += (raw_string.data[j + 1] - 'a') + 10;
                                        }
                                        
                                        else if (raw_string.data[j + 1] >= 'A' && raw_string.data[j + 1] <= 'F')
                                        {
                                            codepoint *= 16;
                                            codepoint += (raw_string.data[j + 1] - 'A') + 10;
                                        }
                                        
                                        else
                                        {
                                            //// ERROR: missing digits in codepoint/byte escape sequence
                                            encountered_errors = true;
                                        }
                                    }
                                    
                                    if (!encountered_errors)
                                    {
                                        if (codepoint <= 0x7F)
                                        {
                                            string.data[string.size++] = (u8)codepoint;
                                        }
                                        
                                        else if (codepoint <= 0x7FF)
                                        {
                                            string.data[string.size++] = 0xC0 | (u8)((codepoint & 0x7C0) >> 6);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x03F) >> 0);
                                        }
                                        
                                        else if (codepoint <= 0xFFFF)
                                        {
                                            string.data[string.size++] = 0xE0 | (u8)((codepoint & 0xF000) >> 12);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x0FC0) >> 6);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x003F) >> 0);
                                        }
                                        
                                        else if (codepoint <= 0x10FFFF)
                                        {
                                            string.data[string.size++] = 0xF0 | (u8)((codepoint & 0x1C0000) >> 18);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x03F000) >> 12);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x000FC0) >> 6);
                                            string.data[string.size++] = 0x80 | (u8)((codepoint & 0x00003F) >> 0);
                                        }
                                        
                                        else
                                        {
                                            //// ERROR: Unicode codepoint out of UTF-8 range
                                            encountered_errors = true;
                                        }
                                    }
                                }
                                
                                else
                                {
                                    switch (raw_string.data[j])
                                    {
                                        case '\"': string.data[string.size++] = '\"'; break;
                                        case '\'': string.data[string.size++] = '\''; break;
                                        case '\\': string.data[string.size++] = '\\'; break;
                                        
                                        case 'a': string.data[string.size++] = '\a'; break;
                                        case 'b': string.data[string.size++] = '\b'; break;
                                        case 'f': string.data[string.size++] = '\f'; break;
                                        case 'n': string.data[string.size++] = '\n'; break;
                                        case 'r': string.data[string.size++] = '\r'; break;
                                        case 't': string.data[string.size++] = '\t'; break;
                                        case 'v': string.data[string.size++] = '\v'; break;
                                        
                                        default:
                                        {
                                            //// ERROR: Unknown escape sequence
                                            encountered_errors = true;
                                        } break;
                                    }
                                }
                            }
                            
                            else string.data[string.size++] = raw_string.data[j];
                        }
                        
                        if (!encountered_errors)
                        {
                            if (c == '"')
                            {
                                token->kind   = Token_String;
                                token->string = string;
                            }
                            
                            else
                            {
                                token->kind = Token_Character;
                                
                                if (string.size == 0)
                                {
                                    //// ERROR: Empty character literal
                                    encountered_errors = true;
                                }
                                
                                else if ((string.data[0] & 0xF0) == 0 && string.size != 1 ||
                                         (string.data[0] & 0xF0) == 0 && string.size != 2 ||
                                         (string.data[0] & 0xF0) == 0 && string.size != 3 ||
                                         (string.data[0] & 0xF0) == 0 && string.size != 4)
                                {
                                    //// ERROR: Character literals may only contain one character
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    token->character = 0;
                                    Copy(string.data, &token->character, string.size);
                                }
                            }
                        }
                    }
                }
                
                else
                {
                    //// ERROR: Unknown symbol
                    encountered_errors = true;
                }
            } break;
        }
        
        umm token_size = lexer.offset - token->offset;
        
        if (token_size <= U16_MAX) token->size = (u16)token_size;
        else
        {
            //// ERROR: Token is too large
            encountered_errors = true;
        }
    }
    
    return !encountered_errors;
}