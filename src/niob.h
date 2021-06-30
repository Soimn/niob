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
typedef u32 Scope_ID;

#define BLANK_IDENTIFIER 0
#define IDENTIFIER_TABLE_BLOCK_SIZE 2048

typedef struct Memory_Arena
{
    struct Memory_Block* first;
    struct Memory_Block* current;
    u32 block_size;
} Memory_Arena;

// IMPORTANT NOTE: Supports only alignment leq. 8 bytes
#define BUCKET_ARRAY_BUCKET_CACHE_SIZE 16
typedef struct Bucket_Array
{
    Memory_Arena* arena;
    struct Bucket* first;
    struct Bucket* current;
    umm bucket_capacity;
    umm current_bucket_size;
    umm bucket_count;
    umm element_size;
    
    struct Bucket* bucket_cache[BUCKET_ARRAY_BUCKET_CACHE_SIZE];
} Bucket_Array;

#define Bucket_Array(T) Bucket_Array
#define BA_INIT(A, T, C) (Bucket_Array){.arena = (A), .element_size = sizeof(T), .bucket_capacity = (C)}

///////////////////////////////////////////////////////////////////////////////

enum EXPRESSION_KIND
{
    // precedence 0: 0 - 19
    Expr_Invalid = 0,
    
    Expr_Identifier,
    Expr_String,
    Expr_Char,
    Expr_Int,
    Expr_Float,
    Expr_Boolean,
    Expr_StructLiteral,
    Expr_ArrayLiteral,
    Expr_Proc,
    Expr_ProcType,
    Expr_Struct,
    Expr_Union,
    Expr_Enum,
    
    // precedence 1: 20 - 39
    Expr_FirstTypeLevel = 20,
    Expr_PointerType,
    Expr_SliceType,
    Expr_ArrayType,
    Expr_DynamicArrayType,
    Expr_PolymorphicType,
    Expr_LastTypeLevel = Expr_PolymorphicType,
    
    // precedence 2: 40 - 59
    Expr_FirstPostfixLevel = 40,
    Expr_Subscript = Expr_FirstPostfixLevel,
    Expr_Slice,
    Expr_Call,
    Expr_ElementOf,
    Expr_PostIncrement,
    Expr_PostDecrement,
    Expr_LastPostfixLevel = Expr_PostDecrement,
    
    // precedence 3: 60 - 79
    Expr_FirstPrefixLevel = 60,
    Expr_Negation = Expr_FirstPrefixLevel,
    Expr_Complement,
    Expr_Not,
    Expr_PreIncrement,
    Expr_PreDecrement,
    Expr_Reference,
    Expr_Dereference,
    Expr_Spread,
    Expr_LastPrefixLevel = Expr_Spread,
    
    // precedence 4: 80 - 99
    Expr_FirstRangeLevel = 80,
    Expr_ClosedRange = Expr_FirstRangeLevel,
    Expr_HalfOpenRange,
    Expr_LastRangeLevel = Expr_HalfOpenRange,
    
    // precedence 5: 100 - 119
    Expr_FirstMulLevel = 100,
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
    
    // precedence 6: 120 - 139
    Expr_FirstAddLevel = 120,
    Expr_Add = Expr_FirstAddLevel,
    Expr_Sub,
    Expr_BitwiseOr,
    Expr_BitwiseXor,
    Expr_LastAddLevel = Expr_BitwiseOr,
    
    // precedence 7: 140 - 159
    Expr_FirstComparative = 140,
    Expr_IsEqual = Expr_FirstComparative,
    Expr_IsNotEqual,
    Expr_IsStrictlyLess,
    Expr_IsStrictlyGreater,
    Expr_IsLess,
    Expr_IsGreater,
    Expr_LastComparative = Expr_IsGreater,
    
    // precedence 8: 160 - 179
    Expr_And = 160,
    
    // precedence 9: 180 - 199
    Expr_Or = 180,
    
    // precedence 10: 200 - 219
    Expr_Conditional,
};

typedef struct Expression
{
    struct Expression* next;
    Enum8(EXPRESSION_KIND) kind;
} Expression;

/////////////////////////////////

typedef struct Argument
{
    struct Argument* next;
	Expression* name;
	Expression* value;
} Argument;

typedef struct Parameter
{
	struct Parameter* next;
	Expression* names;
	Expression* type;
	Expression* value;
	bool is_using;
} Parameter;

typedef struct Return_Value
{
	struct Return_Value* next;
	Expression* name;
	Expression* type;
} Return_Value;

typedef struct Struct_Member
{
	struct Struct_Member* next;
	Expression* type;
	Identifier name;
	bool is_using;
} Struct_Member;

typedef struct Enum_Member
{
	struct Enum_Member* next;
	Identifier name;
	Expression* value;
} Enum_Member;

/////////////////////////////////

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

typedef struct Conditional_Expression
{
    struct Expression;
    
    Expression* condition;
    Expression* true_branch;
    Expression* false_branch;
} Conditional_Expression;

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
    Argument* arguments;
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
    u64 integer;
    f64 floating;
} BasicLiteral_Expression;

typedef struct StructLiteral_Expression
{
    struct Expression;
    
    Expression* type;
    Argument* arguments;
    
} StructLiteral_Expression;

typedef struct ArrayLiteral_Expression
{
    struct Expression;
    
    Expression* type;
    Argument* arguments;
} ArrayLiteral_Expression;

typedef struct Proc_Expression
{
    struct Expression;
    
    Parameter* parameters;
    Return_Value* return_values;
    Expression* polymorph_condition;
    struct Block_Statement* body;
    bool is_decl;
} Proc_Expression;

typedef struct Struct_Expression
{
    struct Expression;
    
    Parameter* parameters;
    Expression* polymorph_condition;
    Struct_Member* members;
    bool is_decl;
} Struct_Expression;

typedef struct Enum_Expression
{
    struct Expression;
    
    Expression* elem_type;
    Enum_Member* members;
    bool is_decl;
} Enum_Expression;
// NOTE: Used for storing expressions on the stack
typedef union Any_Expression
{
    Unary_Expression unary_expression;
    Binary_Expression binary_expression;
    ArrayType_Expression arraytype_expression;
    Subscript_Expression subscript_expression;
    Slice_Expression slice_expression;
    Call_Expression call_expression;
    ElementOf_Expression elementof_expression;
    BasicLiteral_Expression basicliteral_expression;
    StructLiteral_Expression structliteral_expression;
    ArrayLiteral_Expression arrayliteral_expression;
    Proc_Expression proc_expression;
    Struct_Expression struct_expression;
    Enum_Expression enum_expression;
} Any_Expression;

enum STATEMENT_KIND
{
    Statement_Invalid = 0,
    
    Statement_Expression,
    Statement_Block,
    Statment_Declaration,
    
    Statement_If,
    Statement_When,
    Statement_While,
    Statement_For,
    Statement_Break,
    Statement_Continue,
    Statement_Defer,
    Statement_Using,
    Statement_Return,
    Statement_Assignment,
    Statement_Unreachable,
    Statement_NotImplemented,
    
    Statement_VariableDecl,
    Statement_ConstantDecl,
    Statement_ImportDecl,
};

typedef struct Statement
{
    struct Statement* next;
    Enum8(STATEMENT_KIND) kind;
} Statement;

typedef struct Expression_Statement
{
    struct Statement;
    
    Expression* expression;
} Expression_Statement;

typedef struct Block_Statement
{
    struct Statement;
    
    bool is_do;
    Scope_ID scope;
    Identifier label;
    Statement* statements;
} Block_Statement;

typedef struct If_Statement
{
    struct Statement;
    
    Identifier label;
    Statement* init;
    Expression* condition;
    Block_Statement true_branch;
    Block_Statement false_branch;
} If_Statement;

typedef struct While_Statement
{
    struct Statement;
    
    Identifier label;
    Statement* init;
    Expression* condition;
    Statement* step;
    Block_Statement body;
} While_Statement;

typedef struct For_Statement
{
    struct Statement;
    
    Identifier label;
    Expression* symbols;
    Expression* collection;
    Block_Statement body;
} For_Statement;

typedef struct ScopeControl_Statement
{
    struct Statement;
    
    Identifier label;
} ScopeControl_Statement;

typedef struct Defer_Statement
{
    struct Statement;
    
    Statement* statement;
} Defer_Statement;

typedef struct Using_Statement
{
    struct Statement;
    
    Expression* symbol;
} Using_Statement;

typedef struct Return_Statement
{
    struct Statement;
    
    Argument* arguments;
} Return_Statement;

typedef struct Assignment_Statement
{
    struct Statement;
    
    Enum8(EXPRESSION_KIND) op;
    Expression* lhs;
    Expression* rhs;
} Assignment_Statement;

typedef struct Variable_Declaration
{
    struct Statement;
    
    Expression* names;
    Expression* type;
    Expression* values;
    bool is_uninitialized;
    bool is_using;
} Variable_Declaration;

typedef struct Constant_Declaration
{
    struct Statement;
    
    Expression* names;
    Expression* type;
    Expression* values;
    bool is_using;
} Constant_Declaration;

typedef struct Import_Declaration
{
    struct Statement;
    
    String path;
    Identifier alias;
} Import_Declaration;

// NOTE: Used for storing statements on the stack
typedef struct Any_Statement
{
    Expression_Statement expression_statement;
    Block_Statement block_statement;
    If_Statement if_statement;
    While_Statement while_statement;
    For_Statement for_statement;
    ScopeControl_Statement scopecontrol_statement;
    Defer_Statement defer_statement;
    Using_Statement using_statement;
    Return_Statement return_statement;
    Assignment_Statement assignment_statement;
    Variable_Declaration variable_declaration;
    Constant_Declaration constant_declaration;
    Import_Declaration import_declaration;
    
} Any_Statement;

enum DECLARATION_KIND
{
    Decl_Invalid = 0,
    
    Decl_Proc,
    Decl_Struct,
    Decl_Union,
    Decl_Enum,
    Decl_Variable,
    Decl_Constant,
    Decl_Import,
};

typedef struct Declaration
{
    Enum8(DECLARATION_KIND) kind;
    Statement* statement;
} Declaration;

///////////////////////////////////////////////////////////////////////////////

typedef struct Workspace
{
    // resources
    
    // store files
    // temp
    // store declarations (buckets)
    // store identifiers (buckets)
    
    Bucket_Array(Identifier) identifier_table;
    
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


void* Arena_PushSize(Memory_Arena* arena, umm size, u8 alignment);
void  Arena_ClearAll(Memory_Arena* arena);
void  Arena_FreeAll(Memory_Arena* arena);

void* BA_Push(Bucket_Array* array);
void  BA_Pop(Bucket_Array* array, void* value);
umm   BA_ElementCount(Bucket_Array* array);
void* BA_ElementAt(Bucket_Array* array, umm index);
void  BA_ClearAll(Bucket_Array* array);

#endif