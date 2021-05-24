niob programming language

for those who like to get off on jargon, this language is a compiled and statically typed procedural systems programming language

Some of my gripes with C (which I would still use, if not for my stupid decision to "improve" it)
 - there is way too much implementation defined and undefined behaviour
 - there is no __standard__ library, only several "standard" libraries
 - lack of scope control (like using and restricted global access)
 - lack of automatic dereferencing (removing the -> operator and using . with automatic deref allows hot-cold splitting of structs without changing usage code)
 - lack of easy to use math primitives, and or, operator overloading
 - hard to parse
 - lack of type checked macros
 - no name string for enum values
 - no compiletime execution
 - null terminated strings
 - no slice/array reference/pointer + size type
 - ...

Why I won't use Odin instead (I done AoC, written a 6502 emulator, some board game solvers and a few visualizers in Odin)
 - the type system is annoying
	(since it has no, or at least few, implicit casts I find myself having to litter "meaningless" casts everywhere, since I cannot communicate the guarantees I have about values, e.g. x is
     an u8, y is an i8, x is always greater than 0 and less than 255, y is between -1 and 1, x + y does not work, since u8 + i8 is not legal, so I have to do u8(i8(x) + y), eventhough I
     have asserted that the guarantees hold. Another example of this is how u64 % 8 is always a u64 and cannot be implicitly casted to an u8, eventhough it is impossible for the value
     to be greater than 7)
 - I hate the new code style errors, since I have much more trouble reading code that is formated in that way (whitespace is immensely important for grouping and distinction)
 - there are also other reasons, but it all boils down to Odin being essentially what I want in a language, but with a whole lot of tedium added

Main goals of niob
 - minimal amount of undefined behaviour (and undefined behaviour should cause a hard crash on debug builds)
 - a type system that works with the programmer, instead of against them (e.g. won't have the programmer litter meaningless casts everywhere, and implicit casts should not be shocking)
 - easy to parse, while still being easy to write and read (i.e. easy to parse, but not lisp)
 - be minimal and "feel like C"
 - allow compile time execution and metaprogramming
 - allow writing code without worrying about memory usage, and then later bundle allocations, reduce footprint and improve locality without major changes to the code
   (e.g. provide a dynamic array type and a default allocator which can be overriden)

Upfront decisions
 - declarations follow name before type (name: type)
 - everything is zero initialized by default, with an option to avoid initialization, or initialize with a specific value
 - only declarations are allowed in global scope
 - global declarations are unordered
 - no goto (although this removes a lot of freedom, it also allows the compiler to guarantee a lot more)

Comments:
// this is a comment

/* this is a block comment /* this is a nested block comment */ */

Declarations:
 - there are two main types of declarations: variables and constants. Variables behave like in C, while constants are compile time constants which "do not exist at runtime" (unless they are
   added to the data segment, which is the case for strings)
 - there are also import and include declarations, include works like #include in C (but it works on the AST, not text) and import is used for libraries
 - "using" declarations are used to import symbols into a scope

// this is a variable declaration with zero initialization
name : type;

// this is a variable declaration with no initialization
name : type = ---;

// this is a variable declaration initialized to the value of an expression
name : type = expression;

// this is a vartiable delaration initialized to the value of an expression, where the type of variable is infered from the expression
name : = expression;

// this is a constant declaration
name : type : expression;

// this is a constant declaration with an infered type
name : : expression;

// constant declarations cannot be zero initialized or uninitialized

// both variable and constant declarations support multiple names and values, but not multiple types

name0, name1, name2 : type;         // legal var decl, equivalent to name0 : type; name1 : type; name2 : type;
name0, name1, name2 : type0, type1; // illegal

name0, name1, name2 : type = expression0, expression1, expression2; // legal var decl, equivalent to name0 : type = expression0; name1 : type = expression1; name2 : type = expression2;
name0, name1, name2 : type : expression0, expression1, expression2; // legal const decl, equivalent to name0 : type : expression0; name1 : type : expression1; name2 : type : expression2;

name0, name1, name2 :  = expression0, expression1, expression2; // legal var decl, equivalent to name0 : = expression0; name1 : = expression1; name2 : = expression2;
name0, name1, name2 :  : expression0, expression1, expression2; // legal const decl, equivalent to name0 : : expression0; name1 : : expression1; name2 : : expression2;

name0, name1, name2 :  = expression; // legal if expression0 provides 3 values (e.g. procedure with three return values)
name0, name1, name2 :  : expression; // legal if expression0 provides 3 values (e.g. procedure with three return values)

// this is an import declaration (the import system will be covered later)
import "import_path";
foreign import "import_path";
include "include_path";

// this is a using declaration
using expression;                 // imports the symbols in expression into the current scope                    (e.g. using a;      // a.x can now be accessed as x  )
using expression0 as expression1; // imports the symbols in expression0 into a new scope accessed by expression1 (e.g. using a as b; // a.x can now be accessed as b.x)

// using also works on other declarations (but renaming with "as" is not allowed on variable and constant declarations)
using name : type;              // equivalent to: name : type; using name;
using name : type : expression; // equivalent to: name : type : expression; using name;

using import "import_path";
using import "import_path" as expression;
using foreign import "import_path";
using foreign import "import_path" as expression;
//using include "include_path";
//using include "include_path" as expression;

Statements:

// this is an if statement
if condition {}
if condition do statement;

// also supports an optional init clause (e.g. works like { init; if (condition) {} } in C)
if init; condition {}
if init; condition do statement;

// with a following else statement
if condition {}
else if condition do statement;
else do statement;

if condition do statement;
else if condition {}
else {};

// this is a when statement (like a #if statement in C, but part of the language. Only one of the branches are type checked, the others are discarded)
when condition do {}
when condition do statement;

// mirrors if statement
when init; condition do {}
when init; condition do statement;

when condition {}
else when condition do statement;
else do statement;

when condition do statement;
else when condition {}
else {};

// PS: mixing if and when is not allowed, i.e. the following is illegal
if condition {}
else when condition {}


// this is a while statement (equivalent to for loop in C)
while init; condition; post {}
while init; condition; post do statement;

// init and post are optional
while condition {}                  // legal
while condition do statement;       // legal
while init; condition {}            // legal
while init; condition do statement; // legal
while condition; post {}            // illegal
while condition; post do statement; // illegal

// break and continue accept labels
break;
break label;
continue;
continue label;

// labels are placed before if, while and blocks
label: if condition do statement;
label: while condition do statement;
label: {}

// break label; can be used in if statements and blocks with no surrounding while loop to break out of the target scope

// return supports no values, multiple values, and named values
return;
return expression;
return expression0, expression1;
return name = expression;
return name0 = expression0, name1 = expression1;

// cleanup with goto is replaced with defer
defer statement;
defer {}

Expressions:

operators (all binary operators are left-associative):
  .,
  slice[:], subscript[], call()
  +, -, &, *, ~, !
  infix call
  *, /, %, &, >>, << (logical right and left shift)
  +, -, |, ^
  ==, !=, <=, >=, <, >
  &&,
  ||,
  ?:

primary expressions:
  idenfifier
  number
  string
  struct literal
  array literal
 

@notes:
 - before declaration statements and scopes, and after fields and parameters

@note name : type;

@note {}

if condition @note {}

struct { field_name: type @note }

proc(parameter_name: type @note)

Metaprogramming:
 - provide language constructs for all local tasks
 - allow modification of the compiler
 - compiler is a library

common use cases:
 - generate code from existing code
 - insert code into existing code
 - check code for compliance to a ruleset

Types:

uint(u64), u8, u16, u32, u64
int(i64), i8, i16, i32, i64
bool(b1), b8, b16, b32, b64
f16, f32, f64

u8-64 implicitly casts to uint
u8-32 implicitly casts to int
i8-64 implicitly casts to int
b8-64 do not implicitly cast to bool