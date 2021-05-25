#ifndef NIOB_H
#define NIOB_H
#ifdef _WIN32

#ifdef NIOB_INTERNAL
# define API_FUNC //__declspec(dllexport)
#else
# define API_FUNC //__declspec(dllimport)
#endif

typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#elif __linux__

#ifdef NIOB_INTERNAL
# define API_FUNC //__attribute__ ((visibility("default")))
#else
# define API_FUNC //__attribute__ ((visibility("default")))
#endif

typedef __INT8_TYPE__  i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef __UINT8_TYPE__  u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;

#else

#error "unsupported platform"

#endif

typedef u64 umm;
typedef i64 imm;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

typedef b8 bool;

#define false 0
#define true 1

#define Enum8(NAME)  u8
#define Enum16(NAME) u16
#define Enum32(NAME) u32
#define Enum64(NAME) u64

///////////////////////////////////////////////////////////////////////////////

typedef struct String
{
    u8* data;
    umm size;
} String;

typedef struct Memory_Arena
{
    struct Memory_Block* first;
    struct Memory_Block* current;
} Memory_Arena;

typedef struct Bucket_Array
{
    Memory_Arena* arena;
    void* first;
    void* current;
    u16 element_size;
    u16 bucket_size;
    u16 current_bucket_size;
    u16 bucket_count;
} Bucket_Array;

#define Bucket_Array(T) Bucket_Array
#define BUCKET_ARRAY_INIT(A, T, B) (Bucket_Array){.arena = (A), .element_size = sizeof(T), .bucket_size = (B)}

typedef struct Bucket_Array_Iterator
{
    void* current_bucket;
    umm current_index;
    void* current;
    u16 element_size;
    u16 bucket_size;
    u16 last_bucket_size;
} Bucket_Array_Iterator;

///////////////////////////////////////////////////////////////////////////////

typedef struct Compiler_State
{
    // TODO: Split into several arenas later, if needed
    Memory_Arena persistent_arena;
    Memory_Arena temp_arena;
} Compiler_State;

typedef struct Error_Report
{
    u32 line;
    u32 column;
    u32 length;
    u32 file_id;
    String message;
} Error_Report;

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

API_FUNC bool LexString(String string, Memory_Arena* token_arena, Memory_Arena* string_arena, Bucket_Array(Token)* tokens, Error_Report* error_report);

#endif