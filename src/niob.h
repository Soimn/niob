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
    u32 default_block_size;
} Memory_Arena;

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

typedef struct Text_Pos
{
    u32 offset_to_line_start;
    u32 offset;
    u32 line;
} Text_Pos;

typedef struct Text
{
    Text_Pos pos;
    u32 length;
} Text;

typedef struct Token
{
    struct Token* next;
    
    Enum32(TOKEN_KIND) kind;
    Text text;
    
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

///////////////////////////////////////////////////////////////////////////////

enum EXPRESSION_KIND
{
    Expr_Invalid = 0,
    
    // ternary
    Expr_Conditional,
    
    // binary
    
    // precedence: 1
    Expr_Or,
    //
    
    // precedence: 2
    Expr_And,
    //
    
    // precedence: 3
    Expr_IsEqual,
    Expr_IsNotEqual,
    Expr_IsLess,
    Expr_IsLessEQ,
    Expr_IsGreater,
    Expr_IsGreaterEQ,
    //
    
    // precedence: 4
    Expr_Add,
    Expr_Sub,
    Expr_BitOr,
    Expr_BitXor,
    //
    
    // precedence: 5
    Expr_Mul,
    Expr_Div,
    Expr_Rem,
    Expr_BitAnd,
    Expr_LShift,
    Expr_RShift,
    Expr_InfixCall,
    Expr_Chain,
    //
    
    Expr_Member,
    
    // unary
    Expr_Plus,
    Expr_Minus,
    Expr_BitNot,
    Expr_Not,
    Expr_Reference,
    Expr_Dereference,
    
    Expr_Call,
    Expr_Slice,
    Expr_Subscript,
    
    // nesting
    Expr_Compound,
    
    // types
    Expr_PointerType,
    Expr_SliceType,
    Expr_ArrayType,
    Expr_DynamicArrayType,
    
    // primary
    Expr_StructLiteral,
    Expr_ArrayLiteral,
    Expr_Proc,
    Expr_Struct,
    Expr_Enum,
    Expr_Identifier,
    Expr_String,
    Expr_Int,
    Expr_Float,
    Expr_Boolean,
};

typedef struct Named_Argument
{
    struct Named_Argument* next;
    struct Expression* name;
    struct Expression* value;
} Named_Argument;

typedef struct Expression
{
    struct Expression* parent;
    struct Expression* prev;
    struct Expression* next;
    
    Enum8(EXPRESSION_KIND) kind;
    Text text;
    
    union
    {
        struct
        {
            struct Expression* condition;
            struct Expression* true_expr;
            struct Expression* false_expr;
        } conditional;
        
        struct
        {
            struct Expression* left;
            struct Expression* right;
        };
        
        struct Expression* operand;
        
        struct
        {
            struct Expression* pointer;
            Named_Argument arguments;
        } call;
        
        struct
        {
            struct Expression* pointer;
            struct Expression* start_index;
            struct Expression* stop_index;
        } slice;
        
        struct
        {
            struct Expression* pointer;
            struct Expression* index;
        } subscript;
        
        struct Expression* compound_expression;
        
        struct
        {
            struct Expression* size;
            struct Expression* elem_type;
        } array_type;
        
        struct
        {
            struct Expression* type;
            Named_Argument arguments;
        } struct_literal, array_literal;
        
        struct
        {
        } proc;
        
        struct
        {
        } structure;
        
        struct
        {
        } enumeration;
        
        String identifier;
        String string;
        u64 integer;
        f64 floating;
        bool boolean;
    };
} Expression;

enum AST_NODE_KIND
{
    AST_Invalid = 0,
    
    AST_Block,
    AST_Expression,
    
    AST_VarDecl,
    AST_ConstDecl,
    AST_ImportDecl,
    AST_IncludeDecl,
    
    AST_If,
    AST_When,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Using,
    AST_Defer,
    AST_Return,
    AST_Assignment,
};

typedef struct AST_Node
{
    struct AST_Node* parent;
    struct AST_Node* prev;
    struct AST_Node* next;
    
    Enum8(AST_NODE_KIND) kind;
    Text text;
    
    union
    {
        struct
        {
            struct AST_Node* first_child;
            struct AST_Node* last_child;
        } block;
        
        Expression* expression;
        
        struct
        {
            Expression* names;
            Expression* type;
            Expression* values;
            bool is_using;
            bool is_uninitialized;
        } variable;
        
        struct
        {
            Expression* names;
            Expression* type;
            Expression* values;
            bool is_using;
        } constant;
        
        struct
        {
            String import_path;
            String alias;
            bool is_foreign;
        } import;
        
        struct
        {
            String include_path;
        } include;
        
        struct
        {
            struct AST_Node* init;
            Expression* condition;
            struct AST_Node* true_statement;
            struct AST_Node* false_statement;
        } if_statement, when_statement;
        
        struct
        {
            struct AST_Node* init;
            Expression* condition;
            struct AST_Node* body;
        } while_statement;
        
        struct
        {
            Expression* label;
        } break_statement, continue_statement;
        
        struct
        {
            Expression* symbol;
        } using_statement;
        
        struct
        {
            struct AST_Node* statement;
        } defer_statement;
        
        struct
        {
            Named_Argument arguments;
        } return_statement;
        
        struct
        {
            Expression* lhs;
            Expression* rhs;
            Enum8(EXPRESSION_KIND) op;
        } assignment_statement;
    };
} AST_Node;

///////////////////////////////////////////////////////////////////////////////

API_FUNC bool LexString(String string, Memory_Arena* token_arena, Memory_Arena* string_arena, Error_Report* error_report, Token** tokens);
API_FUNC bool ParseTokens(Token* tokens, Memory_Arena* ast_arena, Memory_Arena* string_arena, Error_Report* error_report, AST_Node** ast);

#endif