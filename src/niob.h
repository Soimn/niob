#ifndef NIOB_H
#define NIOB_H
#ifdef _WIN32

typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#elif __linux__

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

typedef u64 Identifier;
typedef u32 Character;

///////////////////////////////////////////////////////////////////////////////

enum EXPRESSION_KIND
{
    // precedence 0: 0 - 12
    Expr_Invalid = 0,
    
    Expr_Identifier,
    Expr_String,
    Expr_Char,
    //Expr_Number
    Expr_Boolean,
    Expr_StructLiteral,
    Expr_ArrayLiteral,
    Expr_Compound,
    
    // precedence 1: 13 - 25
    Expr_FirstPostfixLevel = 13,
    Expr_Subscript = Expr_FirstPostfixLevel,
    Expr_Slice,
    Expr_Call,
    Expr_ElementOf,
    Expr_PostIncrement,
    Expr_PostDecrement,
    Expr_LastPostfixLevel = Expr_PostDecrement,
    
    // precedence 2: 26 - 38
    Expr_FirstPrefixLevel = 26,
    Expr_Negation = Expr_FirstPrefixLevel,
    Expr_Complement,
    Expr_Not,
    Expr_PreIncrement,
    Expr_PreDecrement,
    Expr_Reference,
    Expr_Dereference,
    Expr_PointerType,
    Expr_SliceType,
    Expr_ArrayType,
    Expr_DynamicArrayType,
    Expr_PolymorphicType,
    Expr_Spread,
    Expr_LastPrefixLevel = Expr_Spread,
    
    // precedence 3: 39 - 51
    Expr_FirstRangeLevel = 39,
    Expr_ClosedRange = Expr_FirstRangeLevel,
    Expr_HalfOpenRange,
    Expr_LastRangeLevel = Expr_HalfOpenRange,
    
    // precedence 4: 52 - 64
    Expr_FirstMulLevel = 52,
    Expr_Mul = Expr_FirstMulLevel,
    Expr_Div,
    Expr_Rem,
    Expr_Mod,
    Expr_BitwiseAnd,
    Expr_ArithmeticRightShift,
    Expr_RightShift,
    Expr_LeftShift,
    Expr_InfixCall,
    Expr_LastMulLevel = Expr_InfixCall,
    
    // precedence 5: 65 - 77
    Expr_FirstAddLevel = 65,
    Expr_Add = Expr_FirstAddLevel,
    Expr_Sub,
    Expr_BitwiseOr,
    Expr_BitwiseXor,
    Expr_LastAddLevel = Expr_BitwiseOr,
    
    // precedence 6: 78 - 90
    Expr_FirstComparative = 78,
    Expr_IsEqual = Expr_FirstComparative,
    Expr_IsNotEqual,
    Expr_IsStrictlyLess,
    Expr_IsStrictlyGreater,
    Expr_IsLess,
    Expr_IsGreater,
    Expr_LastComparative = Expr_IsGreater,
    
    // precedence 7: 91 - 103
    Expr_And = 91,
    
    // precedence 8: 104 - 116
    Expr_Or = 104,
};

typedef struct Expression
{
    Enum8(EXPRESSION_KIND) kind;
} Expression;

typedef struct Unary_Expression
{
    struct Expression;
    
    Expression* operand;
} Unary_Expression;

typedef struct Binary_Expression
{
    struct Expression;
    
    Expression* left;
    Expression* right;
} Binary_Expression;

typedef struct InfixCall_Expression
{
    struct Expression;
    
    Identifier procedure;
    Expression* left;
    Expression* right;
} InfixCall_Expression;

typedef struct ArrayType_Expression
{
    struct Expression;
    
    Expression* type;
    Expression* size;
} ArrayType_Expression;

typedef struct Subscript_Expression
{
    struct Expression;
    
    Expression* array;
    Expression* index;
} Subscript_Expression;

typedef struct Slice_Expression
{
    struct Expression;
    
    Expression* array;
    Expression* start;
    Expression* end;
} Slice_Expression;

typedef struct Call_Expression
{
    struct Expression;
    
    Expression* pointer;
    // arguments
} Call_Expression;

typedef union BasicLiteral_Expression
{
    struct Expression;
    
    Identifier identifier;
    String string;
    Character character;
    bool boolean;
    // number
} BasicLiteral_Expression;

typedef struct StructLiteral_Expression
{
    struct Expression;
    
    Expression* type;
    // arugments
    
} StructLiteral_Expression;

typedef struct ArrayLiteral_Expression
{
    struct Expression;
    
    Expression* type;
    // arguments
} ArrayLiteral_Expression;

enum STATEMENT_KIND
{
    Statement_Invalid = 0,
    
    Statement_Expression,
    Statement_Assignment,
    
    // ...
};

typedef struct Statement
{
    Enum8(STATEMENT_KIND) kind;
} Statement;

///////////////////////////////////////////////////////////////////////////////

typedef struct Workspace
{
    // resources
    
    // hash table of identifiers
    
    // unchecked declarations
    // committed declarations
    
    // error buffer
} Workspace;

typedef struct Declaration
{
    
} Declaration;

Workspace* WS_Open(void);
void WS_Close(Workspace* workspace);

void WS_AddFile(Workspace* workspace, String file_path);

bool WS_RequestDeclaration(Workspace* workspace, Declaration* declaration);
void WS_ResubmitDeclaration(Workspace* workspace, Declaration declaration);
void WS_CommitDeclaration(Workspace* workspace, Declaration declaration);

void WS_GenerateCode(Workspace* workspace);

void WS_FlushErrorBuffer(Workspace* workspace);

#endif