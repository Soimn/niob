// META NOTE: This seems somewhat stupidly overcomplicated for a lexer, but I don't see any huge flaws with it
// NOTE: The enum value for any symbol is it's ASCII value, where multi symbol tokens take the value
//       of the first token pluss the sum of 2 times the ASCII values for the succeeding symbols*
//
//       * Token_Arrow and Token_ThickArrow are the only exceptions, where the succeeding symbols'
//         sum is multiplied by 3 instead of 2
enum TOKEN_KIND
{
    Token_Invalid = 0,
    Token_EndOfStream,
    
    Token_Plus            = '+',
    Token_PlusEqual       = '+' + 2*'=',
    Token_Minus           = '-',
    Token_MinusEqual      = '-' + 2*'=',
    Token_Arrow           = '-' + 3*'>',
    Token_Star            = '*',
    Token_StarEqual       = '*' + 2*'=',
    Token_Slash           = '/',
    Token_SlashEqual      = '/' + 2*'=',
    Token_Percentage      = '%',
    Token_PercentageEqual = '%' + 2*'=',
    
    Token_Less                = '<',
    Token_LessEqual           = '<' + 2*'=',
    Token_LessLess            = '<' + 2*'<',
    Token_LessLessEqual       = '<' + 2*'<' + 2*'=',
    Token_Greater             = '>',
    Token_GreaterEqual        = '>' + 2*'=',
    Token_GreaterGreater      = '>' + 2*'>',
    Token_GreaterGreaterEqual = '>' + 2*'>' + 2*'=',
    
    Token_BitNot      = '~',
    Token_Not         = '!',
    Token_NotEqual    = '!' + 2*'=',
    Token_BitAnd      = '&',
    Token_BitAndEqual = '&' + 2*'=',
    Token_And         = '&' + 2*'&',
    Token_AndEqual    = '&' + 2*'&' + 2*'=',
    Token_BitOr       = '|',
    Token_BitOrEqual  = '|' + 2*'=',
    Token_Or          = '|' + 2*'|',
    Token_OrEqual     = '|' + 2*'|' + 2*'=',
    Token_Hat         = '^',
    Token_HatEqual    = '^' + 2*'=',
    
    Token_Equal       = '=',
    Token_EqualEqual  = '=' + 2*'=',
    Token_ThickArrow  = '=' + 3*'>',
    
    Token_Period            = '.',
    Token_PeriodPeriod      = '.' + 2*'.',
    Token_PeriodPeriodLess  = '.' + 2*'.' + 2*'<',
    Token_Colon             = ':',
    Token_Comma             = ',',
    Token_Semicolon         = ';',
    
    Token_Cash              = '$',
    Token_Question          = '?',
    Token_At                = '@',
    Token_Pound             = '#',
    Token_Underscore        = '_',
    
    Token_OpenBrace         = '{',
    Token_CloseBrace        = '}',
    Token_OpenBracket       = '[',
    Token_CloseBracket      = ']',
    Token_OpenParen         = '(',
    Token_CloseParen        = ')',
    
    Token_Identifier = 495,
    Token_String,
    Token_Character,
    Token_Int,
    Token_Float,
    
    Token_Comment,
};

enum KEYWORD_KIND
{
    Keyword_Invalid = 0,
    
    Keyword_Proc,
    Keyword_Where,
    Keyword_Struct,
    Keyword_Union,
    Keyword_Enum,
    Keyword_If,
    Keyword_Else,
    Keyword_While,
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    Keyword_True,
    Keyword_False,
    Keyword_Do,
    
    KEYWORD_COUNT
};

typedef struct Token
{
    Enum32(TOKEN_KIND) kind;
    u32 line;
    u32 offset_to_line_start;
    u32 offset;
    u32 size;
    
    union
    {
        struct
        {
            String string;
            Enum8(KEYWORD_KIND) keyword;
        };
        
        u64 integer;
        f64 floating;
        u32 character;
    };
} Token;

typedef struct Lexer
{
    u8* start;
    Token current_token;
    Token peek_token;
} Lexer;

Token
Lexer_GetNextToken(Lexer* lexer, Token prev_token)
{
    Token token             = {0};
    bool encountered_errors = false;
    
    u32 offset               = prev_token.offset;
    u32 line                 = prev_token.line;
    u32 offset_to_line_start = prev_token.offset_to_line_start;
    
    while (true)
    {
        if (lexer->start[offset] == '\n')
        {
            offset_to_line_start = offset;
            offset += 1;
            line   += 1;
        }
        
        else if (lexer->start[offset] == ' '  ||
                 lexer->start[offset] == '\r' ||
                 lexer->start[offset] == '\t' ||
                 lexer->start[offset] == '\v' ||
                 lexer->start[offset] == '\f')
        {
            offset += 1;
        }
        
        else break;
    }
    
    token.line                 = line;
    token.offset_to_line_start = offset_to_line_start;
    token.offset               = offset;
    token.size                 = 0;
    
    if (lexer->start[offset] == 0) token.kind = Token_EndOfStream;
    else
    {
        if (lexer->start[offset] == '/' && (lexer->start[offset + 1] == '/' || lexer->start[offset + 1] == '*'))
        {
            token.kind = Token_Comment;
            
            offset += 1;
            
            if (lexer->start[offset] == '/')
            {
                offset += 1;
                
                umm offset_to_start_of_comment = offset;
                
                while (lexer->start[offset] != 0 && lexer->start[offset] != '\n')
                {
                    offset += 1;
                }
                
                token.string.data = lexer->start + offset_to_start_of_comment;
                token.string.size = offset - offset_to_start_of_comment;
                
                if (lexer->start[offset] == '\n')
                {
                    offset_to_line_start = offset;
                    offset += 1;
                    line   += 1;
                }
            }
            
            else
            {
                offset += 1;
                
                umm offset_to_start_of_comment = offset;
                
                while (lexer->start[offset] != 0 && (lexer->start[offset] != '*' || lexer->start[offset + 1] != '/'))
                {
                    if (lexer->start[offset] == '\n')
                    {
                        offset_to_line_start = offset;
                        line += 1;
                    }
                    
                    offset += 1;
                }
                
                token.string.data = lexer->start + offset_to_start_of_comment;
                token.string.size = offset - offset_to_start_of_comment;
                
                if (lexer->start[offset] == 0)
                {
                    //// ERROR: missing end of block comment
                    encountered_errors = true;
                }
                
                else offset += 2;
            }
        }
        
        else if (lexer->start[offset] == '+' ||
                 lexer->start[offset] == '*' ||
                 lexer->start[offset] == '/' ||
                 lexer->start[offset] == '&' ||
                 lexer->start[offset] == '^' ||
                 lexer->start[offset] == '!')
        {
            token.kind = lexer->start[offset];
            
            offset += 1;
            
            if (lexer->start[offset] == '=')
            {
                offset += 1;
                
                token.kind += 2*'=';
            }
        }
        
        else if (lexer->start[offset] == '<' ||
                 lexer->start[offset] == '>' ||
                 lexer->start[offset] == '&' ||
                 lexer->start[offset] == '|')
        {
            token.kind = lexer->start[offset];
            
            offset += 1;
            
            if (lexer->start[offset] == token.kind)
            {
                offset += 1;
                
                token.kind += 2*lexer->start[offset];
            }
            
            if (lexer->start[offset] == '=')
            {
                offset += 1;
                
                token.kind += 2*'=';
            }
        }
        
        else if (lexer->start[offset] == '-')
        {
            token.kind = lexer->start[offset];
            
            offset += 1;
            
            if (lexer->start[offset] == '=')
            {
                offset += 1;
                
                token.kind += 2*'=';
            }
            
            else if (lexer->start[offset] == '>')
            {
                offset += 1;
                
                token.kind += '>';
            }
        }
        
        
        else if (lexer->start[offset] == '-' || lexer->start[offset] == '=')
        {
            token.kind = lexer->start[offset];
            
            offset += 1;
            
            if (lexer->start[offset] == '=')
            {
                offset += 1;
                
                token.kind += 2*'=';
            }
            
            else if (lexer->start[offset] == '>')
            {
                offset += 1;
                
                token.kind += 3*'>';
            }
        }
        
        else if (lexer->start[offset] == ':' ||
                 lexer->start[offset] == ',' ||
                 lexer->start[offset] == ';' ||
                 lexer->start[offset] == '~' ||
                 lexer->start[offset] == '$' ||
                 lexer->start[offset] == '?' ||
                 lexer->start[offset] == '@' ||
                 lexer->start[offset] == '#' ||
                 lexer->start[offset] == '{' ||
                 lexer->start[offset] == '}' ||
                 lexer->start[offset] == '[' ||
                 lexer->start[offset] == ']' ||
                 lexer->start[offset] == '(' ||
                 lexer->start[offset] == ')')
        {
            token.kind = lexer->start[offset];
            
            offset += 1;
        }
        
        else if (lexer->start[offset] >= 'a' && lexer->start[offset] <= 'z' ||
                 lexer->start[offset] >= 'A' && lexer->start[offset] <= 'Z' ||
                 lexer->start[offset] == '_')
        {
            if (!(lexer->start[offset + 1] >= 'a' && lexer->start[offset + 1] <= 'z' ||
                  lexer->start[offset + 1] >= 'A' && lexer->start[offset + 1] <= 'Z' ||
                  lexer->start[offset + 1] == '_'))
            {
                token.kind = Token_Underscore;
            }
            
            else
            {
                umm offset_to_start_of_identifier = offset;
                
                while (lexer->start[offset] >= 'a' && lexer->start[offset] <= 'z' ||
                       lexer->start[offset] >= 'A' && lexer->start[offset] <= 'Z' ||
                       lexer->start[offset] == '_')
                {
                    offset += 1;
                }
                
                token.string.data = lexer->start + offset_to_start_of_identifier;
                token.string.size = offset - offset_to_start_of_identifier;
                token.keyword     =  Keyword_Invalid;
                
                String KeywordStrings[KEYWORD_COUNT] = {
                    [Keyword_Invalid]  = (String){0},
                    [Keyword_Proc]     = CONST_STRING("proc"),
                    [Keyword_Where]    = CONST_STRING("where"),
                    [Keyword_Struct]   = CONST_STRING("struct"),
                    [Keyword_Union]    = CONST_STRING("union"),
                    [Keyword_Enum]     = CONST_STRING("enum"),
                    [Keyword_If]       = CONST_STRING("if"),
                    [Keyword_Else]     = CONST_STRING("else"),
                    [Keyword_While]    = CONST_STRING("while"),
                    [Keyword_Break]    = CONST_STRING("break"),
                    [Keyword_Continue] = CONST_STRING("continue"),
                    [Keyword_Using]    = CONST_STRING("using"),
                    [Keyword_Defer]    = CONST_STRING("defer"),
                    [Keyword_Return]   = CONST_STRING("return"),
                    [Keyword_True]     = CONST_STRING("true"),
                    [Keyword_False]    = CONST_STRING("false"),
                    [Keyword_Do]       = CONST_STRING("do"),
                };
                
                for (umm i = 0; i < ARRAY_SIZE(KeywordStrings); ++i)
                {
                    String s0 = token.string;
                    String s1 = KeywordStrings[i];
                    
                    while (s0.size != 0 && s1.size != 0 && *s0.data == *s1.data)
                    {
                        s0.data += 1;
                        s0.size -= 1;
                        s1.data += 1;
                        s1.size -= 1;
                    }
                    
                    // NOTE: Strings match
                    if (s0.size == 0 && s1.size == 0)
                    {
                        token.keyword = (u8)i;
                    }
                }
            }
        }
        
        else if (lexer->start[offset] == '.' || lexer->start[offset] >= '0' && lexer->start[offset] <= '9')
        {
            if (lexer->start[offset] == '.' && !(lexer->start[offset] >= '0' && lexer->start[offset] <= '9'))
            {
                offset += 1;
                
                if (lexer->start[offset] == '.')
                {
                    offset += 1;
                    
                    token.kind += 2*'.';
                    
                    if (lexer->start[offset] == '<')
                    {
                        offset += 1;
                        
                        token.kind += 2*'<';
                    }
                }
                
                else token.kind = Token_Period;
            }
            
            else
            {
                bool is_hex    = false;
                bool is_binary = false;
                bool is_float  = false;
                
                if (lexer->start[offset] == '0')
                {
                    if      (lexer->start[offset] == 'x') is_hex = true;
                    else if (lexer->start[offset] == 'b') is_binary = true;
                    else if (lexer->start[offset] == 'h') is_hex = true, is_float = true;
                    
                    if (is_hex || is_binary) offset += 2;
                }
                
                else if (lexer->start[offset] == '.')
                {
                    is_float = true;
                    offset  += 1;
                }
                
                u64 integer            = 0;
                f64 floating           = 0;
                i64 exponent           = 0;
                umm base               = (is_hex ? 16 : (is_binary ? 2 : 10));
                umm digit_count        = 0;
                bool integer_overflow  = false;
                f64 floating_place     = (is_float ? 0.1 : 1);
                bool exponent_overflow = false;
                
                while (!encountered_errors)
                {
                    if (lexer->start[offset] == '.')
                    {
                        if (is_float) break;
                        else
                        {
                            is_float = true;
                        }
                    }
                    
                    else if (lexer->start[offset] == '_')
                    {
                        if (lexer->start[offset - 1] == '.' ||
                            lexer->start[offset - 1] == 'x' ||
                            lexer->start[offset - 1] == 'h' ||
                            lexer->start[offset - 1] == 'b')
                        {
                            //// ERROR: encountered digit separator with no preceeding digit
                            encountered_errors = true;
                        }
                        
                        else offset += 1;
                    }
                    
                    else if (lexer->start[offset] == 'e')
                    {
                        if (is_hex || is_binary)
                        {
                            //// ERROR: exponents are not alowed on binary and hexadecimal literals
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            is_float = true;
                            
                            offset += 1;
                            
                            i8 sign = 1;
                            if      (lexer->start[offset] == '+') offset += 1;
                            else if (lexer->start[offset] == '-')
                            {
                                sign    = -1;
                                offset += 1;
                            }
                            
                            if (!(lexer->start[offset] >= '0' && lexer->start[offset] <= '9'))
                            {
                                //// ERROR: missing exponent
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                while (lexer->start[offset] >= '0' && lexer->start[offset] <= '9')
                                {
                                    i64 old_exponent = exponent;
                                    exponent         = exponent * 10 + (lexer->start[offset] - '0');
                                    
                                    exponent_overflow = exponent_overflow || (old_exponent > exponent);
                                    offset += 1;
                                }
                            }
                            
                            exponent *= sign;
                        }
                    }
                    
                    else
                    {
                        u8 digit = 0;
                        
                        if (lexer->start[offset] >= '0' && lexer->start[offset] <= '9')
                        {
                            digit = lexer->start[offset] - '0';
                            
                            if (is_binary && digit > 1)
                            {
                                //// ERROR: Invalid digit in binary literal
                                encountered_errors = true;
                            }
                            
                            else offset += 1;
                        }
                        
                        else if (is_hex && (lexer->start[offset] >= 'a' && lexer->start[offset] <= 'f' ||
                                            lexer->start[offset] >= 'A' && lexer->start[offset] <= 'F'))
                        {
                            digit   = 10 + (lexer->start[offset] - (lexer->start[offset] >= 'A' ? 'A' : 'a'));
                            offset += 1;
                        }
                        
                        else break;
                        
                        u64 old_integer = integer;
                        
                        integer  = integer * base + digit;
                        floating = (is_float ? floating : floating * base) + digit * floating_place;
                        
                        if (old_integer > integer) integer_overflow = true;
                        if (is_float)              floating_place  /= 10;
                        digit_count += 1;
                    }
                }
                
                if (lexer->start[offset - 1] == '_')
                {
                    //// ERROR: encountered digit separator with no succeeding digit
                    encountered_errors = true;
                }
                
                else
                {
                    if (is_float)
                    {
                        if (is_hex)
                        {
                            if (digit_count == 4)
                            {
                                u32 int32   = (u32)integer;
                                f32 float32 = 0;
                                Copy(&int32, &float32, sizeof(u32));
                                
                                token.kind     = Token_Float;
                                token.floating = (f64)float32;
                            }
                            
                            else if (digit_count == 8)
                            {
                                f64 float64 = 0;
                                Copy(&integer, &float64, sizeof(u64));
                                
                                token.kind     = Token_Float;
                                token.floating = float64;
                            }
                            
                            else
                            {
                                //// ERROR: invalid number of digits in hexadecimal float literal
                                encountered_errors = true;
                            }
                        }
                        
                        else
                        {
                            token.kind     = Token_Float;
                            token.floating = floating;
                        }
                    }
                    
                    else
                    {
                        if (integer_overflow)
                        {
                            //// ERROR: integer literal is too large
                            encountered_errors = true;
                        }
                        
                        else
                        {
                            token.kind    = Token_Int;
                            token.integer = integer;
                        }
                    }
                }
            }
        }
        
        else if (lexer->start[offset] == '"' || lexer->start[offset] == '\'')
        {
            u8 terminator = lexer->start[offset];
            offset += 1;
            
            umm offset_to_start_of_string = offset;
            
            bool contains_escaped_characters = false;
            while (lexer->start[offset] != 0 && lexer->start[offset] != terminator)
            {
                if (lexer->start[offset] == '\\' && lexer->start[offset] != 0)
                {
                    contains_escaped_characters = true;
                    offset += 1;
                }
                
                offset += 1;
            }
            
            String raw_string;
            raw_string.data = lexer->start + offset_to_start_of_string;
            raw_string.size = offset - offset_to_start_of_string;
            
            String string;
            if (!contains_escaped_characters) string = raw_string;
            else
            {
                string.data = 0; NOT_IMPLEMENTED;
                string.size = 0;
                
                for (umm i = 0; i < raw_string.size && !encountered_errors; ++i)
                {
                    if (raw_string.data[i] == '\\')
                    {
                        i += 1;
                        
                        if (raw_string.data[i] == 'x' || raw_string.data[i] == 'u')
                        {
                            umm digit_count = (raw_string.data[i] == 'x' ? 2 : 6);
                            
                            u32 codepoint = 0;
                            for (umm j = 0; j < digit_count; ++j, ++i)
                            {
                                u8 c = (raw_string.size > i + 1 ? raw_string.data[i + 1] : 0);
                                
                                u8 digit = 0;
                                if      (c >= '0' && c <= '9') digit = c - '0';
                                else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
                                else if (c >= 'A' && c <= 'F') digit = 10 + (c - 'A');
                                else
                                {
                                    //// ERROR: missing digits in escape sequence
                                    encountered_errors = true;
                                    break;
                                }
                                
                                codepoint = codepoint * 16 + digit;
                            }
                            
                            if (!encountered_errors)
                            {
                                if (codepoint <= U8_MAX)
                                {
                                    string.data[string.size++] = (u8)codepoint;
                                }
                                
                                else if (codepoint <= 0x7FF)
                                {
                                    string.data[string.size++] = (u8)(0xC0 | (codepoint & 0x7C0) >> 6);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x03F) >> 0);
                                }
                                
                                else if (codepoint <= 0xFFFF)
                                {
                                    string.data[string.size++] = (u8)(0xE0 | (codepoint & 0xF000) >> 12);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x0FC0) >> 6);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x003F) >> 0);
                                }
                                
                                else if (codepoint <= 0x10FFFF)
                                {
                                    string.data[string.size++] = (u8)(0xF0 | (codepoint & 0x1C0000) >> 18);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x03F000) >> 12);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x000FC0) >> 6);
                                    string.data[string.size++] = (u8)(0x80 | (codepoint & 0x00003F) >> 0);
                                }
                                
                                else
                                {
                                    //// ERROR: Codepoint is out of UTF-8 range
                                    encountered_errors = true;
                                }
                            }
                        }
                        
                        else
                        {
                            u8 insert = 0;
                            switch (raw_string.data[i])
                            {
                                case 'a':  insert = '\a'; break;
                                case 'b':  insert = '\b'; break;
                                case 'f':  insert = '\f'; break;
                                case 'n':  insert = '\n'; break;
                                case 'r':  insert = '\r'; break;
                                case 't':  insert = '\t'; break;
                                case 'v':  insert = '\v'; break;
                                case '"':  insert = '"';  break;
                                case '\\': insert = '\\'; break;
                                case '\'': insert = '\''; break;
                                default:
                                {
                                    //// ERROR: unknown escape sequence
                                    encountered_errors = true;
                                } break;
                            }
                            
                            string.data[string.size] = insert;
                            string.size += 1;
                        }
                    }
                    
                    else
                    {
                        string.data[string.size] = raw_string.data[i];
                        string.size += 1;
                    }
                }
            }
            
            if (terminator == '"')
            {
                token.kind   = Token_String;
                token.string = string;
            }
            
            else
            {
                if (string.size == 1 && (*string.data & 0x80) == 0    ||
                    string.size == 2 && (*string.data & 0xE0) == 0xA0 ||
                    string.size == 3 && (*string.data & 0xF0) == 0xE0 ||
                    string.size == 4 && (*string.data & 0xF8) == 0xF0)
                {
                    token.kind = Token_Character;
                    Copy(string.data, &token.character, string.size);
                }
                
                else
                {
                    //// ERROR: character literals may only contain one unicode codepoint
                    encountered_errors = true;
                }
            }
        }
        
        else
        {
            //// ERROR: Invalid symbol
            encountered_errors = true;
        }
    }
    
    token.size = offset - token.offset;
    
    if (encountered_errors)
    {
        token.kind = Token_Invalid;
    }
    
    return token;
}

Lexer
LexString(String string)
{
    // NOTE: The lexer only works on null terminated strings
    ASSERT(string.data[string.size - 1] == 0);
    
    Lexer lexer = {0};
    
    lexer.current_token = Lexer_GetNextToken(&lexer, (Token){0});
    lexer.peek_token    = Lexer_GetNextToken(&lexer, lexer.current_token);
    
    return lexer;
}

Token
GetToken(Lexer* lexer)
{
    return lexer->current_token;
}

Token
PeekNextToken(Lexer* lexer)
{
    return lexer->peek_token;
}

void
SkipPastToken(Lexer* lexer, Token token)
{
    // NOTE: Make sure the token passed is not older than the current token, or not provided by the lexer
    ASSERT(MemoryCompare(&token, &lexer->current_token, sizeof(Token)) ||
           MemoryCompare(&token, &lexer->peek_token, sizeof(Token)));
    
    ASSERT(token.kind != Token_Invalid);
    
    lexer->current_token = lexer->peek_token;
    lexer->peek_token    = Lexer_GetNextToken(lexer, lexer->current_token);
}