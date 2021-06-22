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

typedef u32 Identifier;
typedef u32 Character;

#define IDENTIFIER_TABLE_BLOCK_SIZE 2048

typedef struct Memory_Arena
{
    struct Memory_Block* first;
    struct Memory_Block* current;
    u32 block_size;
} Memory_Arena;

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
    Expr_Proc,
    Expr_Struct,
    Expr_Union,
    Expr_Enum,
    
    // precedence 1: 13 - 25
    Expr_FirstTypeLevel = 13,
    Expr_PointerType,
    Expr_SliceType,
    Expr_ArrayType,
    Expr_DynamicArrayType,
    Expr_PolymorphicType,
    Expr_LastTypeLevel = Expr_PolymorphicType,
    
    // precedence 2: 26 - 38
    Expr_FirstPostfixLevel = 26,
    Expr_Subscript = Expr_FirstPostfixLevel,
    Expr_Slice,
    Expr_Call,
    Expr_ElementOf,
    Expr_PostIncrement,
    Expr_PostDecrement,
    Expr_LastPostfixLevel = Expr_PostDecrement,
    
    // precedence 3: 39 - 51
    Expr_FirstPrefixLevel = 39,
    Expr_Negation = Expr_FirstPrefixLevel,
    Expr_Complement,
    Expr_Not,
    Expr_PreIncrement,
    Expr_PreDecrement,
    Expr_Reference,
    Expr_Dereference,
    Expr_Spread,
    Expr_LastPrefixLevel = Expr_Spread,
    
    // precedence 4: 52 - 64
    Expr_FirstRangeLevel = 52,
    Expr_ClosedRange = Expr_FirstRangeLevel,
    Expr_HalfOpenRange,
    Expr_LastRangeLevel = Expr_HalfOpenRange,
    
    // precedence 5: 65 - 77
    Expr_FirstMulLevel = 65,
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
    
    // precedence 6: 78 - 90
    Expr_FirstAddLevel = 78,
    Expr_Add = Expr_FirstAddLevel,
    Expr_Sub,
    Expr_BitwiseOr,
    Expr_BitwiseXor,
    Expr_LastAddLevel = Expr_BitwiseOr,
    
    // precedence 7: 91 - 103
    Expr_FirstComparative = 91,
    Expr_IsEqual = Expr_FirstComparative,
    Expr_IsNotEqual,
    Expr_IsStrictlyLess,
    Expr_IsStrictlyGreater,
    Expr_IsLess,
    Expr_IsGreater,
    Expr_LastComparative = Expr_IsGreater,
    
    // precedence 8: 104 - 116
    Expr_And = 104,
    
    // precedence 9: 117 - 129
    Expr_Or = 117,
    
    Expr_ArgumentAssignment,
};

typedef struct Expression
{
    struct Expression* next;
    Enum8(EXPRESSION_KIND) kind;
} Expression;

typedef struct Named_Argument
{
    Expression* next;
    Expression* name;
    Expression* value;
} Named_Argument;

typedef struct Parameter
{
    Expression* next;
    Expression* names;
    Expression* type;
    Expression* values;
    bool is_using;
} Parameter;

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
    Named_Argument* arguments;
} Call_Expression;

typedef struct ElementOf_Expression
{
    struct Expression;
    
    Expression* left;
    Identifier right;
} ElementOf_Expression;

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
    Named_Argument* arguments;
    
} StructLiteral_Expression;

typedef struct ArrayLiteral_Expression
{
    struct Expression;
    
    Expression* type;
    Named_Argument* arguments;
} ArrayLiteral_Expression;

typedef struct Proc_Expression
{
    struct Expression;
    
    Parameter* parameters;
    Parameter* return_values;
    struct Statement* body;
} Proc_Expression;

typedef struct Struct_Expression
{
    struct Expression;
    
    Parameter* parameters;
    // members
} Struct_Expression;

typedef struct Union_Expression
{
    struct Expression;
    
    Parameter* parameters;
    // members
} Union_Expression;

typedef struct Enum_Expression
{
    struct Expression;
    
    Expression* elem_type;
    // members
} Enum_Expression;

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

enum DECLARATION_KIND
{
    Decl_Import,
    Decl_Variable,
    Decl_Constant,
    Decl_Proc,
    Decl_Struct,
    Decl_Union,
    Decl_Enum,
};

typedef struct Declaration
{
    Enum8(DECLARATION_KIND) kind;
} Declaration;

///////////////////////////////////////////////////////////////////////////////

typedef struct Workspace
{
    // resources
    
    Memory_Arena identifier_table;
    
    // unchecked declarations
    // committed declarations
    
    // error buffer
} Workspace;

Workspace* WS_Open(void);
void WS_Close(Workspace* workspace);

void WS_AddFile(Workspace* workspace, String file_path);

bool WS_RequestDeclaration(Workspace* workspace, Declaration* declaration);
void WS_ResubmitDeclaration(Workspace* workspace, Declaration declaration);
void WS_CommitDeclaration(Workspace* workspace, Declaration declaration);

void WS_GenerateCode(Workspace* workspace);

void WS_FlushErrorBuffer(Workspace* workspace);
Identifier WS_GetIdentifier(Workspace* workspace, String string);

#endif