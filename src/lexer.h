void
Lexer_ReportError(Error_Report* error_report, u32 line, u32 offset_to_line_start, u32 offset, u32 error_length, String message)
{
    *error_report = (Error_Report){
        .line    = line,
        .column  = (offset - offset_to_line_start) + 1,
        .length  = error_length,
        .message = message,
    };
}

API_FUNC bool
LexString(String string, Memory_Arena* token_arena, Memory_Arena* string_arena, Bucket_Array(Token)* tokens, Error_Report* error_report)
{
    ASSERT(string.size >= 1 && string.data[string.size - 1] == 0);
    
    ASSERT(tokens->arena == 0 && tokens->current == 0);
    *tokens = BUCKET_ARRAY_INIT(token_arena, Token, 256);
    
    bool encountered_errors = false;
    
    u8* content              = string.data;
    u32 offset_to_line_start = 0;
    u32 offset               = 0;
    u32 line                 = 1;
    
    for(;;)
    {
        Token* token = BucketArray_Append(tokens);
        
        for (;;)
        {
            if (content[offset] == '\n')
            {
                offset_to_line_start = offset;
                offset += 1;
                line   += 1;
            }
            
            else if (content[offset] == ' '  ||
                     content[offset] == '\r' ||
                     content[offset] == '\t' ||
                     content[offset] == '\v' ||
                     content[offset] == '\f')
            {
                offset += 1;
            }
            
            else break;
        }
        
        token->line                 = line;
        token->offset_to_line_start = offset_to_line_start;
        token->offset               = offset;
        token->size                 = 0;
        
        if (content[offset] == 0) token->kind = Token_EndOfStream;
        else
        {
            if (content[offset] == '/' && (content[offset + 1] == '/' || content[offset + 1] == '*'))
            {
                token->kind = Token_Comment;
                
                offset += 1;
                
                if (content[offset] == '/')
                {
                    offset += 1;
                    
                    umm offset_to_start_of_comment = offset;
                    
                    while (content[offset] != 0 && content[offset] != '\n')
                    {
                        offset += 1;
                    }
                    
                    token->string.data = content + offset_to_start_of_comment;
                    token->string.size = offset - offset_to_start_of_comment;
                    
                    if (content[offset] == '\n')
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
                    
                    while (content[offset] != 0 && (content[offset] != '*' || content[offset + 1] != '/'))
                    {
                        if (content[offset] == '\n')
                        {
                            offset_to_line_start = offset;
                            line += 1;
                        }
                        
                        offset += 1;
                    }
                    
                    token->string.data = content + offset_to_start_of_comment;
                    token->string.size = offset - offset_to_start_of_comment;
                    
                    if (content[offset] == 0)
                    {
                        //// ERROR: missing end of block comment
                        Lexer_ReportError(error_report, token->line, token->offset_to_line_start, token->offset, 2, CONST_STRING("block comment has no end"));
                        encountered_errors = true;
                    }
                    
                    else offset += 2;
                }
            }
            
            else if (content[offset] == '+' ||
                     content[offset] == '*' ||
                     content[offset] == '/' ||
                     content[offset] == '%' ||
                     content[offset] == '&' ||
                     content[offset] == '^' ||
                     content[offset] == '!')
            {
                token->kind = content[offset];
                
                offset += 1;
                
                if (content[offset] == '=')
                {
                    offset += 1;
                    
                    token->kind += 2*'=';
                }
            }
            
            else if (content[offset] == '<' ||
                     content[offset] == '>' ||
                     content[offset] == '&' ||
                     content[offset] == '|')
            {
                token->kind = content[offset];
                offset += 1;
                
                if (content[offset] == token->kind)
                {
                    token->kind += 2*content[offset];
                    offset += 1;
                }
                
                if (content[offset] == '=')
                {
                    offset += 1;
                    
                    token->kind += 2*'=';
                }
            }
            
            else if (content[offset] == '-')
            {
                token->kind = content[offset];
                
                offset += 1;
                
                if (content[offset] == '=')
                {
                    offset += 1;
                    
                    token->kind += 2*'=';
                }
                
                else if (content[offset] == '>')
                {
                    offset += 1;
                    
                    token->kind += 3*'>';
                }
            }
            
            
            else if (content[offset] == '-' || content[offset] == '=')
            {
                token->kind = content[offset];
                
                offset += 1;
                
                if (content[offset] == '=')
                {
                    offset += 1;
                    
                    token->kind += 2*'=';
                }
                
                else if (content[offset] == '>')
                {
                    offset += 1;
                    
                    token->kind += 3*'>';
                }
            }
            
            else if (content[offset] == ':' ||
                     content[offset] == ',' ||
                     content[offset] == ';' ||
                     content[offset] == '~' ||
                     content[offset] == '$' ||
                     content[offset] == '?' ||
                     content[offset] == '@' ||
                     content[offset] == '#' ||
                     content[offset] == '{' ||
                     content[offset] == '}' ||
                     content[offset] == '[' ||
                     content[offset] == ']' ||
                     content[offset] == '(' ||
                     content[offset] == ')')
            {
                token->kind = content[offset];
                
                offset += 1;
            }
            
            else if (content[offset] >= 'a' && content[offset] <= 'z' ||
                     content[offset] >= 'A' && content[offset] <= 'Z' ||
                     content[offset] == '_')
            {
                if (content[offset] == '_' && !(content[offset + 1] >= 'a' && content[offset + 1] <= 'z' ||
                                                content[offset + 1] >= 'A' && content[offset + 1] <= 'Z' ||
                                                content[offset + 1] == '_'))
                {
                    token->kind = Token_Underscore;
                }
                
                else
                {
                    token->kind = Token_Identifier;
                    
                    umm offset_to_start_of_identifier = offset;
                    
                    while (content[offset] >= 'a' && content[offset] <= 'z' ||
                           content[offset] >= 'A' && content[offset] <= 'Z' ||
                           content[offset] == '_')
                    {
                        offset += 1;
                    }
                    
                    token->string.data = content + offset_to_start_of_identifier;
                    token->string.size = offset - offset_to_start_of_identifier;
                    token->keyword     =  Keyword_Invalid;
                    
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
                        String s0 = token->string;
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
                            token->keyword = (u8)i;
                        }
                    }
                }
            }
            
            else if (content[offset] == '.' || content[offset] >= '0' && content[offset] <= '9')
            {
                if (content[offset] == '.' && !(content[offset] >= '0' && content[offset] <= '9'))
                {
                    offset += 1;
                    
                    if (content[offset] == '.')
                    {
                        offset += 1;
                        
                        token->kind += 2*'.';
                        
                        if (content[offset] == '<')
                        {
                            offset += 1;
                            
                            token->kind += 2*'<';
                        }
                    }
                    
                    else token->kind = Token_Period;
                }
                
                else
                {
                    bool is_hex    = false;
                    bool is_binary = false;
                    bool is_float  = false;
                    
                    if (content[offset] == '0')
                    {
                        if      (content[offset] == 'x') is_hex = true;
                        else if (content[offset] == 'b') is_binary = true;
                        else if (content[offset] == 'h') is_hex = true, is_float = true;
                        
                        if (is_hex || is_binary) offset += 2;
                    }
                    
                    else if (content[offset] == '.')
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
                        if (content[offset] == '.')
                        {
                            if (is_float) break;
                            else
                            {
                                is_float       = true;
                                floating_place = 0.1;
                                
                                offset  += 1;
                            }
                        }
                        
                        else if (content[offset] == '_')
                        {
                            if (content[offset - 1] == '.' ||
                                content[offset - 1] == 'x' ||
                                content[offset - 1] == 'h' ||
                                content[offset - 1] == 'b')
                            {
                                //// ERROR: encountered digit separator with no preceeding digit
                                Lexer_ReportError(error_report, line, offset_to_line_start, offset, 1,
                                                  CONST_STRING("encountered digit separator with no preceeding digit"));
                                encountered_errors = true;
                            }
                            
                            else offset += 1;
                        }
                        
                        else if (content[offset] == 'e')
                        {
                            if (is_hex || is_binary)
                            {
                                //// ERROR: exponents are not alowed on binary and hexadecimal literals
                                Lexer_ReportError(error_report, line, offset_to_line_start, offset, 1,
                                                  CONST_STRING("exponents are not alowed on binary and hexadecimal literals"));
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                is_float = true;
                                
                                offset += 1;
                                
                                i8 sign = 1;
                                if      (content[offset] == '+') offset += 1;
                                else if (content[offset] == '-')
                                {
                                    sign    = -1;
                                    offset += 1;
                                }
                                
                                if (!(content[offset] >= '0' && content[offset] <= '9'))
                                {
                                    //// ERROR: missing exponent
                                    Lexer_ReportError(error_report, line, offset_to_line_start, offset, 1,
                                                      CONST_STRING("missing value of exponent after 'e' suffix in float literal suffix"));
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    while (content[offset] >= '0' && content[offset] <= '9')
                                    {
                                        i64 old_exponent = exponent;
                                        exponent         = exponent * 10 + (content[offset] - '0');
                                        
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
                            
                            if (content[offset] >= '0' && content[offset] <= '9')
                            {
                                digit = content[offset] - '0';
                                
                                if (is_binary && digit > 1)
                                {
                                    //// ERROR: Invalid digit in binary literal
                                    Lexer_ReportError(error_report, line, offset_to_line_start, offset, 1, CONST_STRING("invalid digit in binary literal"));
                                    encountered_errors = true;
                                }
                                
                                else offset += 1;
                            }
                            
                            else if (is_hex && (content[offset] >= 'a' && content[offset] <= 'f' ||
                                                content[offset] >= 'A' && content[offset] <= 'F'))
                            {
                                digit   = 10 + (content[offset] - (content[offset] >= 'A' ? 'A' : 'a'));
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
                    
                    if (content[offset - 1] == '_')
                    {
                        //// ERROR: encountered digit separator with no succeeding digit
                        Lexer_ReportError(error_report, line, offset_to_line_start, offset - 1, 1, CONST_STRING("encountered digit separator with no succeeding digit"));
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
                                    
                                    token->kind     = Token_Float;
                                    token->floating = (f64)float32;
                                }
                                
                                else if (digit_count == 8)
                                {
                                    f64 float64 = 0;
                                    Copy(&integer, &float64, sizeof(u64));
                                    
                                    token->kind     = Token_Float;
                                    token->floating = float64;
                                }
                                
                                else
                                {
                                    //// ERROR: invalid number of digits in hexadecimal float literal
                                    Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, offset - token->offset,
                                                      CONST_STRING("invalid number of digits in hexadecimal float literal (only 4 and 8 digit literals are allowed)"));
                                    encountered_errors = true;
                                }
                            }
                            
                            else
                            {
                                token->kind     = Token_Float;
                                token->floating = floating;
                                
                                if (exponent != 0)
                                {
                                    // HACK
                                    
                                    f64 old_floating = token->floating;
                                    
                                    if (exponent < 0) for (umm i = 0; i < (umm)-exponent; ++i) token->floating /= 10;
                                    else              for (umm i = 0; i < (umm) exponent; ++i) token->floating *= 10;
                                    
                                    u64 floating_bits;
                                    Copy(&token->floating, &floating_bits, sizeof(f64));
                                    
                                    if (token->floating == 0 && old_floating != 0  ||
                                        floating_bits == 0x7FF0000000000000        ||
                                        floating_bits == 0xFFF0000000000000        ||
                                        floating_bits == 0x7FF0000000000001        ||
                                        floating_bits == 0x7FF8000000000001        ||
                                        floating_bits == 0x7FFFFFFFFFFFFFFF)
                                    {
                                        //// ERROR: exponent is too magnificent
                                        Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, offset - token->offset,
                                                          (exponent < 0 ? CONST_STRING("exponent is too small") : CONST_STRING("exponent is too large")));
                                        encountered_errors = true;
                                    }
                                }
                            }
                        }
                        
                        else
                        {
                            if (integer_overflow)
                            {
                                //// ERROR: integer literal is too large
                                Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, offset - token->offset,
                                                  CONST_STRING("integer literal is too large to be represented by any integer type"));
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                token->kind    = Token_Int;
                                token->integer = integer;
                            }
                        }
                    }
                }
            }
            
            else if (content[offset] == '"' || content[offset] == '\'')
            {
                u8 terminator = content[offset];
                offset += 1;
                
                umm offset_to_start_of_string = offset;
                bool contains_escaped_characters = false;
                while (content[offset] != 0 && content[offset] != terminator)
                {
                    if (content[offset] == '\\' && content[offset] != 0)
                    {
                        contains_escaped_characters = true;
                        offset += 1;
                    }
                    offset += 1;
                }
                
                if (content[offset] == 0)
                {
                    //// ERROR: unterminated string/character literal
                    Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, 1,
                                      (terminator == '"' ? CONST_STRING("unterminated string literal") : CONST_STRING("unterminated character literal")));
                    encountered_errors = true;
                }
                
                else
                {
                    String raw_string;
                    raw_string.data = content + offset_to_start_of_string;
                    raw_string.size = offset - offset_to_start_of_string;
                    
                    offset += 1;
                    
                    String resolved_string;
                    if (!contains_escaped_characters) resolved_string = raw_string;
                    else
                    {
                        resolved_string.data = Arena_PushSize(string_arena, raw_string.size, ALIGNOF(u8));
                        resolved_string.size = 0;
                        
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
                                            Lexer_ReportError(error_report, line, offset_to_line_start, (u32)(offset_to_start_of_string + i - (j + 2)), (u32)(j + 2),
                                                              CONST_STRING("missing digits in escape sequence"));
                                            encountered_errors = true;
                                            break;
                                        }
                                        
                                        codepoint = codepoint * 16 + digit;
                                    }
                                    
                                    if (!encountered_errors)
                                    {
                                        if (codepoint <= U8_MAX)
                                        {
                                            resolved_string.data[resolved_string.size++] = (u8)codepoint;
                                        }
                                        
                                        else if (codepoint <= 0x7FF)
                                        {
                                            resolved_string.data[resolved_string.size++] = (u8)(0xC0 | (codepoint & 0x7C0) >> 6);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x03F) >> 0);
                                        }
                                        
                                        else if (codepoint <= 0xFFFF)
                                        {
                                            resolved_string.data[resolved_string.size++] = (u8)(0xE0 | (codepoint & 0xF000) >> 12);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x0FC0) >> 6);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x003F) >> 0);
                                        }
                                        
                                        else if (codepoint <= 0x10FFFF)
                                        {
                                            resolved_string.data[resolved_string.size++] = (u8)(0xF0 | (codepoint & 0x1C0000) >> 18);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x03F000) >> 12);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x000FC0) >> 6);
                                            resolved_string.data[resolved_string.size++] = (u8)(0x80 | (codepoint & 0x00003F) >> 0);
                                        }
                                        
                                        else
                                        {
                                            //// ERROR: Codepoint is out of UTF-8 range
                                            Lexer_ReportError(error_report, line, offset_to_line_start, (u32)(offset_to_start_of_string + i - (digit_count + 2)), (u32)(digit_count + 2),
                                                              CONST_STRING("codepoint is out of UTF-8 range"));
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
                                            Lexer_ReportError(error_report, line, offset_to_line_start, (u32)(offset_to_start_of_string + i - 1), 2,
                                                              CONST_STRING("unknown escape sequence"));
                                            encountered_errors = true;
                                        } break;
                                    }
                                    
                                    resolved_string.data[resolved_string.size] = insert;
                                    resolved_string.size += 1;
                                }
                            }
                            
                            else
                            {
                                resolved_string.data[resolved_string.size] = raw_string.data[i];
                                resolved_string.size += 1;
                            }
                        }
                    }
                    
                    if (terminator == '"')
                    {
                        token->kind   = Token_String;
                        token->string = resolved_string;
                    }
                    
                    else
                    {
                        if (resolved_string.size == 1 && (*resolved_string.data & 0x80) == 0    ||
                            resolved_string.size == 2 && (*resolved_string.data & 0xE0) == 0xA0 ||
                            resolved_string.size == 3 && (*resolved_string.data & 0xF0) == 0xE0 ||
                            resolved_string.size == 4 && (*resolved_string.data & 0xF8) == 0xF0)
                        {
                            token->kind = Token_Character;
                            Copy(resolved_string.data, &token->character, resolved_string.size);
                        }
                        
                        else
                        {
                            //// ERROR: character literals must contain exactly one unicode codepoint
                            Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, offset - token->offset,
                                              CONST_STRING("character literals must contain exactly one unicode codepoint"));
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            else
            {
                //// ERROR: Invalid symbol
                Lexer_ReportError(error_report, line, offset_to_line_start, token->offset, 1,
                                  CONST_STRING("encountered an invalid symbol"));
                encountered_errors = true;
            }
        }
        
        token->size = offset - token->offset;
        
        if (encountered_errors) token->kind = Token_Invalid;
        
        if (token->kind == Token_Invalid || token->kind == Token_EndOfStream) break;
    }
    
    return !encountered_errors;
}

#ifdef NIOB_DEBUG
void
DEBUG_PrintTokens(String string_to_lex)
{
    Memory_Arena scratch = {0};
    Bucket_Array(Token) tokens;
    
    Error_Report report;
    if (!LexString(string_to_lex, &scratch, &scratch, &tokens, &report))
    {
        printf("(%d:%d) %.*s\n", report.line, report.column, (int)report.message.size, report.message.data);
    }
    
    else
    {
        for (Bucket_Array_Iterator it = BucketArray_Iterate(&tokens);
             it.current;
             BucketArray_AdvanceIterator(&it))
        {
            Token token = *(Token*)it.current;
            
            char* token_name = 0;
            
            switch (token.kind)
            {
                case Token_Invalid:             token_name = "Token_Invalid";             break;
                case Token_EndOfStream:         token_name = "Token_EndOfStream";         break;
                case Token_Plus:                token_name = "Token_Plus";                break;
                case Token_PlusEqual:           token_name = "Token_PlusEqual";           break;
                case Token_Minus:               token_name = "Token_Minus";               break;
                case Token_MinusEqual:          token_name = "Token_MinusEqual";          break;
                case Token_Arrow:               token_name = "Token_Arrow";               break;
                case Token_Star:                token_name = "Token_Star";                break;
                case Token_StarEqual:           token_name = "Token_StarEqual";           break;
                case Token_Slash:               token_name = "Token_Slash";               break;
                case Token_SlashEqual:          token_name = "Token_SlashEqual";          break;
                case Token_Percentage:          token_name = "Token_Percentage";          break;
                case Token_PercentageEqual:     token_name = "Token_PercentageEqual";     break;
                case Token_Less:                token_name = "Token_Less";                break;
                case Token_LessEqual:           token_name = "Token_LessEqual";           break;
                case Token_LessLess:            token_name = "Token_LessLess";            break;
                case Token_LessLessEqual:       token_name = "Token_LessLessEqual";       break;
                case Token_Greater:             token_name = "Token_Greater";             break;
                case Token_GreaterEqual:        token_name = "Token_GreaterEqual";        break;
                case Token_GreaterGreater:      token_name = "Token_GreaterGreater";      break;
                case Token_GreaterGreaterEqual: token_name = "Token_GreaterGreaterEqual"; break;
                case Token_BitNot:              token_name = "Token_BitNot";              break;
                case Token_Not:                 token_name = "Token_Not";                 break;
                case Token_NotEqual:            token_name = "Token_NotEqual";            break;
                case Token_BitAnd:              token_name = "Token_BitAnd";              break;
                case Token_BitAndEqual:         token_name = "Token_BitAndEqual";         break;
                case Token_And:                 token_name = "Token_And";                 break;
                case Token_AndEqual:            token_name = "Token_AndEqual";            break;
                case Token_BitOr:               token_name = "Token_BitOr";               break;
                case Token_BitOrEqual:          token_name = "Token_BitOrEqual";          break;
                case Token_Or:                  token_name = "Token_Or";                  break;
                case Token_OrEqual:             token_name = "Token_OrEqual";             break;
                case Token_Hat:                 token_name = "Token_Hat";                 break;
                case Token_HatEqual:            token_name = "Token_HatEqual";            break;
                case Token_Equal:               token_name = "Token_Equal";               break;
                case Token_EqualEqual:          token_name = "Token_EqualEqual";          break;
                case Token_ThickArrow:          token_name = "Token_ThickArrow";          break;
                case Token_Period:              token_name = "Token_Period";              break;
                case Token_PeriodPeriod:        token_name = "Token_PeriodPeriod";        break;
                case Token_PeriodPeriodLess:    token_name = "Token_PeriodPeriodLess";    break;
                case Token_Colon:               token_name = "Token_Colon";               break;
                case Token_Comma:               token_name = "Token_Comma";               break;
                case Token_Semicolon:           token_name = "Token_Semicolon";           break;
                case Token_Cash:                token_name = "Token_Cash";                break;
                case Token_Question:            token_name = "Token_Question";            break;
                case Token_At:                  token_name = "Token_At";                  break;
                case Token_Pound:               token_name = "Token_Pound";               break;
                case Token_Underscore:          token_name = "Token_Underscore";          break;
                case Token_OpenBrace:           token_name = "Token_OpenBrace";           break;
                case Token_CloseBrace:          token_name = "Token_CloseBrace";          break;
                case Token_OpenBracket:         token_name = "Token_OpenBracket";         break;
                case Token_CloseBracket:        token_name = "Token_CloseBracket";        break;
                case Token_OpenParen:           token_name = "Token_OpenParen";           break;
                case Token_CloseParen:          token_name = "Token_CloseParen";          break;
                case Token_Identifier:          token_name = "Token_Identifier";          break;
                case Token_String:              token_name = "Token_String";              break;
                case Token_Character:           token_name = "Token_Character";           break;
                case Token_Int:                 token_name = "Token_Int";                 break;
                case Token_Float:               token_name = "Token_Float";               break;
                case Token_Comment:             token_name = "Token_Comment";             break;
            }
            
            if      (token.kind == Token_Int)   printf("(%.2d:%.2d) %s %llu\n", token.line, token.offset - token.offset_to_line_start, token_name, token.integer);
            else if (token.kind == Token_Float) printf("(%.2d:%.2d) %s %f\n", token.line, token.offset - token.offset_to_line_start, token_name, token.floating);
            else                                printf("(%.2d:%.2d) %s\n", token.line, token.offset - token.offset_to_line_start, token_name);
            
            if (token.kind == Token_Invalid || token.kind == Token_EndOfStream) break;
        }
    }
}
#endif