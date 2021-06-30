# The Niob Programming Language

**NOTE:**
This document is a rough outline of the design of the Niob programming langaugen used to aid
development, it is not meant to be a complete introduction to the language, nor an explanation of the
rationale behind the design.

The Niob programming language is a procedural systems programming language for game development.
Niob is inspired by the languages [Odin](https://odin-lang.org/) and [Jai](https://www.youtube.com/playlist?list=PLmV5I2fxaiCKfxMBrNsU1kgKJXD3PkyxO), as well as Casey
Muratori's comments on programming languages during his streams of [Handmade Hero](https://handmadehero.org/).

**NOTE:**
The reason I, the author of Niob, have started work on my own language, instead of using the
aforementioned languages is more out of necessity than anything else. Odin and Jai are both
perfectly viable languages for game development, however I am displeased with a large portion
of their design and especially the direction they are headed. Since there are no other
formidable options for my own use, I am left with no other option than to design it myself.
The language is therefore very opinionated and is __not__ meant to fit into any preexisting
paradigm or fad. Niob will __never__ support OOP, RAII, "memory safety" or similar concepts.
Finally, the language aims to serve as a replacement to C that is well defined, low friction,
and allows arbitrary levels of abstraction.


## Goals
 - no undefined behaviour
 - minimal and well documented platform dependent behaviour
 - designed around modern 64bit processors
 - empower the programmer and stay out of the way
 - be simple and extensible

## Platform Dependent Behaviour
 - the result and possible exceptions raised from dividing an integer by 0
 - the result and possible exceptions raised from dividing INT_MIN by -1
 - exceptions raised by illegal/exceptional floating point operations
 - whether an exception is raised, or not, when a null pointer is dereferenced
   (depends on the zero page being reserved, which is the case on all major desktop operating
    systems, but usually not on embedded systems)

## Types
```
uint, u8, u16, u32, u64 // unsigned integers
int,  i8, i16, i32, i64 // signed integers
bool, b8, b16, b32, b64 // boolean
float, f32, f64         // floating point

rawptr                  // pure memory address
typid                   // the type of types
any                     // typeid + rawptr
```

## Keywords
```
do
in
proc
struct
union
enum
where
if
when
else
while
for
break
continue
return
defer
using
true
false
```

## Primary expressions
```
identifier                           - /(?<!\w)(_|[A-Za-z])\w*/

binary literal                       - /(?<!\w)0b([0-1]_+[0-1]|[0-1])+/

decimal literal                      - /(?<!\w)0d(\d_+\d|\d)+|(0(?!(x|b|h|d))\d+|[1-9]+)(e[+-]?(\d_+\d|\d)+)/

hexadecimal literal                  - /(?<!\w)0x([\dA-Fa-f]_+[\dA-Fa-f]|[\dA-Fa-f])+/

hexadecimal floating point literal   - /(?<!\w)0h([\dA-Fa-f]_+[\dA-Fa-f]|[\dA-Fa-f])+/

floating point literal               - /(?<!\w)(\d_+\d|\d)*\.(\d_+\d|\d)+(e[+-]?(\d_+\d|\d)+)?/

string literal                       - /"((\\.)|[^"])*"/

character literal                    - '((\\.)|[^'])*'

boolean literal
 true
 false

struct literal
 type.{field0 = elem0, elem1, field2 = elem2}
 .{field0 = elem0, elem1, field2 = elem2}

array literal
 type.[range0 = elem0, elem1, elem2, range1 = elem3]
 .[range0 = elem0, elem1, elem2, range1 = elem3]

compound expression
 (expression)

struct
 struct { field0: type0; field1, field2: type1; }
 struct(param0, param1: param_type0, param2: param_type1) { field0: type0; field1, field2: type1; }

enum
 enum { A, B, C = expression0, D }
 enum elem_type { A, B, C = expression0, D }

procedure
 proc do body;
 proc {}
 proc -> return_type do body;
 proc -> return_type {}
 proc -> (return0: return_type0, return1: return_type1) do body;
 proc ->(return0: return_type0, return1: return_type1) {}
 proc(param0: type0, param1, param2 := value0, param3: type1) do body;
 proc(param0: type0, param1, param2 := value0, param3: type1) {}
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type do body;
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type {}
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type where expression do body;
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type where expression {}
 proc(param0: type0, param1, param2 := value0, param3: type1) -> (return0: return_type0, return1: return_type1) do body;
 proc(param0: type0, param1, param2 := value0, param3: type1) -> (return0: return_type0, return1: return_type1) {}
 proc(param0: type0, param1, param2 := value0, param3: type1) -> (return0: return_type0, return1: return_type1) where expression do body;
 proc(param0: type0, param1, param2 := value0, param3: type1) -> (return0: return_type0, return1: return_type1) where expression {}

procedure type
 proc
 proc -> return_type
 proc -> (return0: return_type0, return1: return_type1)
 proc(param0: type0, param1, param2 := value0, param3: type1)
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type
 proc(param0: type0, param1, param2 := value0, param3: type1) -> return_type
 proc(param0: type0, param1, param2 := value0, param3: type1) -> (return0: return_type0, return1: return_type1)
```

## Operators (grouped by precedence high - low)
```
// postfix unary
 [y]    array subscript        arrays, slices, dynamic arrays
 [y:z]  array slice            arrays, slices, dynamic arrays
 (y, z) procedure call         procedure pointer
 .y     element accessor       struct, union, enum, arrays
 ++     post increment         integers
 --     post decrement         integers

// prefix unary
 +    plus                     integers, floating point, arrays
 ++   increment                integers
 -    negation                 integers, floating point, arrays
 --   decrement                integers
 *    dereference              pointers
 &    reference                addressable value
 ~    bitwise not              integers
 !    logial not               boolean
 ^    pointer type             typeid
 []   slice type               typeid
 [x]  array type               typeid
 [..] dynamic array type       typeid
 $    polymorphic type         symbol
 ..   spread                   arrays

// binary
 ..  closed range              integers
 ..< half open range           integers

// binary
 *     product (binary)        integers, floating point, arrays
 /     quotient                integers, floating point
 /     type slice              typeid
 %     trunc. div. modulo      integers
 %%    floor. div. modulo      integers
 &     bitwise and (binary)    integers
 <<    logical left shift      integers
 >>    logical right shift     integers
 >>>   arithmetic right shift  integers
 ident infix procedure call    all types

 + sum  (binary)               integers, floating point, arrays
 - difference (binary)         integers, floating point, arrays
 | bitwise or                  integers
 ^ bitwise xor                 integers

 == compare equal              integers, floating point, arrays, boolean, pointers
 != compare inequal            integers, floating point, arrays, boolean, pointers
 <  compare less               integers, floating point, arrays, boolean, pointers
 <= compare less or equal      integers, floating point, arrays, boolean, pointers
 >  compare greater            integers, floating point, arrays, boolean, pointers
 >= compare greater or equal   integers, floating point, arrays, boolean, pointers

 && logical and                boolean

 || logical or                 boolean
```

## Statements
block
```
{ statements; }
do statement; // only allowed as body of if, when, for, procedures

// optional label before statement
label: { statements; }
```

if/when
```
// runtime if
if condition do statement;
if condition {}

// compile time if (does not type check false branch)
when condition do statement;
when condition {}

// can be followed by an else clause
if condition do statement;
else         do statement;

// optional label before statement, and init statement (can be an expression, assignment or declaration)
label: if init; condition do statement;
```

while
```
while condition do statement;
while condition {}

// optional label before statement, and init, step statements
// init can be an expression, assignment or declaration, is executed before the loop
// step can be an expression or assignment, is executed after every iteration
label: while init; condition; step do statement;
```

for
```
for symbol in collection do statement;
for symbol in collection {}

// may in some cases allow several symbols
for symbol0, symbol1, symbol2 in collection do statement;

// optional label before statement
label: for symbol in collection do statement;
```

break/continue
```
break;
continue;

// labeled break/continue
break label;
continue label;
```

using
```
using symbol;
```

defer
```
defer statement;
defer {}
```

return
```
return;
return expression;
return expression0, expression1;
return symbol = expression;
return symbol0 = expression0, symbol1 = expression1;
return expression0, symbol1 = expression1;
```

assignment
```
x = y;
x, y = y, x;

// assigment may use any binary operator (except element accessor, infix call, type slice and comparison operators)
x op= y; // e.g. x += y;
```

## Import system
The point of an import system is to create a standard way of encapsulating code. The import system achieves this by defining
the concept of an abstract unit of code (e.g. a bundle of source files, or maybe a subset of a single file), and defines rules
for how these units may interact and the information they either show or hide from other units. Ideally one would be able to
define rules for interaction between units on a per pair basis. This would enable granular control over how information is
shared between units. The problem with this approach is that it the complexity may quickly spiral out of control if the
sharing rules are not carefully monitored. This complexity may be warranted, but it can easily creep into the codebase itself.
This may not be objectively true, but in my opinion, the rules for how the systems, contained in these units of code, interact
is way more important than how the codebase is structured. Giving up control over information sharing rules and adopting a
single standard ruleset for sharing units of code would reduce the complexity imposed by the organization of the codebase.
This would in turn highlight complexities in these systems, since the ambient complexity is reduced.
In the case of defining the "unit of code" the import system governs, there aleady exists two units in all operating systems:
the file and the directory. These units serve to encapsulate data, and may in some cases define rules regarding access rights.
Separating a codebase across different files and directories serves to limit the information displayed to the programmer and
organize it such that information about related subjects are located close to eachother. Files and directories used in this
way can aid the programmer by reducing noise and allowing easy access to relevant information. The problem with files and
directories is that, by themselves, they do not define any rules that govern the visibility of information to code. To remedy
this, a usual approach is to define an abstract unit that is based on either files or directories, and provides additional
rules. This unit will be refered to as a "package". Python is an example of a language that bases packages of files, while Odin
bases packages of directories. An important observation is that basing the package of files removes to ability to segregate the
package into several focused views, unless not every file is a package, which allows segregation but increases complexity. Basing
the package of a directory allows segeregation into files, but introduces some problems regarding the possibility of subdirectories
and which subset of files in the directory should be part of the package. Despite the problems with basing the package of the
directory, it allows a greater degree of freedom, and the problems should be solvable. Regarding subdirectories, if the package
is defined to contain every file in a directory (recursively), it would be impossible to define a master and sub-package
relationship, which is useful when making larger libraries with clearly defined modules. Additionally, if subdirectories are treated
as sub-packages, it would be impossible to distinguish between a package and sub-package, which could cause addressing problems.
Regarding the subset of files treated as part of the language, the extension could be used to distinguish between source files and
other files, however this is not necessarily portable, since not all operating systems rely on file extensions (and adding them
would then seem alien). Since subdirectories are ignored, it would make sense to treat every file in a directory as part of that
package, and requiring other files to be located in subdirectories. The definition of what is contained in a package is therefore
every file which is an immediate child of the package's directory. Now onto the ruleset governing packages. For packages to be
useful they need to define rules regarding visibility, addressing and how this affects the ABI. Visibility is reasonably simple,
since it essentially comes down to segregating the package into a public and private segment. However, there are several ways of
segregating the package. One way is to divide the entire content of the package into two segments, public and private. Another
option is to divide the package into three segments: public, private and file private. This would allow files to hide information
from other parts of the package, as well as other packages. However, introducing the notion of file private information would
essentially elevate files to package status, and could easily be emulated by defining sub-packages. Regarding addressing, by
assigning each package a unique name, the contents of the package could be addressed in the same manner as members of structures
are accessed.And when it comes to linking names, by prefixing each symbol with the package name, it would eliminate all symbol
collisions, as long as every package name is unique. A natural way sepcifying the package name is by naming the directory, however
this wouls allow package names which are not identifiers. To remedy this and make it clearer which files are included in the package,
each file should start with declaring which package they are part of. The last step is to define how dependencies work. There are two
options: allow passing on dependencies, and keep all dendencies private. Since packages are supposed to be encapsulated units of code,
it would make more sense for dependencies to be private, since this restricts the set of visible packages to those imported in a
package's source code.

## Foreign system
automatic dynamic linking

///////////////////////////////////////////////////////////////////

when os == .Windows do foreign import "path/lib.dll";
else                do foreign import "path/liba.so" as lib;

foreign lib
{
	a :: proc ---
	b :: proc ---
	c :: proc ---
	d :: proc ---
	e :: proc ---
	f :: proc ---
}

///////////////////////////////////////////////////////////////////

when os == .Windows
{
	foreign import "path/lib.dll"
	{
		a :: proc ---
		b :: proc ---
		c :: proc ---
		d :: proc ---
		e :: proc ---
		f :: proc ---
	}
}

else
{
	foreign import "path/liba.so"
	{
		a :: proc ---
		b :: proc ---
		c :: proc ---
		d :: proc ---
		e :: proc ---
		f :: proc ---
	}
}

///////////////////////////////////////////////////////////////////



## Attributes, directives, notes and metaprogramming

Symbols at disposal
* @
* #