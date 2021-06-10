# Niob Language Specification

The Niob programming language is a procedural systems programming language that aims to replace C for game programming.

Goals:
 - reflect hardware as closely as possible
 - absolute clarity without needless verbosity
 - maximal control to the programmer
 - extensible and metaprogrammable

The language draws a lot of inspiration from the languages Jai and Odin, as well as the comments by Casey Muratori on
programming languages during his live streams working on Handmade Hero. The rest is mostly the authors disgust of how
undefined behaviour has creeped into every little nook and cranny of the garbage pile that is the C specification.

## The problem with C
There are countless problems with C in regards to the abstracted view of programming language design. Although this language
tries to resolve those problems, they are not the primary concern. The major problem with C is that compilers are written
against the C specification, which is not in line with common practices for C programming. This leads to a lot of
disagreements between compiler authors and users of the language, as compilers have recently broken a lot of code that has
worked for decades by being more agressive with optimization and taking advantage of undefined behaviour. It has clearly
been shown through the usage of C, that the specification is not a good fit for actual software development, and a large
part of this problem is the undefined behaviour. 

What the Niob programming language aims to do is to resolve all undefined behaviour introduced in C. This does not only require
a new language, but also a new optimizing backend, as LLVM takes advantage of a lot of undefined behaviour in C. A significant
portion of the undefined behaviour, listed in appendix J of the ISO/IEC 9899:201x n1570 publicly available draft, of the C11
standard can easily be dealt with by throwing away the standard library and removing the notion of multiple encapsulated
translation units. What is left is a small list of undefined behaviour that concerns integer and floating point operations,
hardware exceptions, calling conventions, evaluation order, unaligned memory access, calling functions by pointer
with wrong arguments and modification of the data segment. There is however a lot of undefined behaviour that is not listed
in this appendix (e.g. infinite loops without side effects are undefined behaviour), and since it would be a monumental, and
honestly pointless, task to identify all implicit undefined behaviour in C, it has been decided that it would be wiser to
focus on making language constructs that avoid those problems altogether.

## Resolving the undefined behaviour
The approach chosen to resolve the undefined behaviour is to first restrict the problem to only relevant architectures, broker between
these architectures to find a common representation that is as close to the actual hardware as possible and then dealing with ways of
expressing architecture dependent behaviour.

### Restricting platform support
Different architectures implement arithmetic operations and exceptions differently, and may also use a different memory model. To broker
between every architecture in existence will quickly lead to the mess that C has made for itself. Since games primarily target desktop,
console and mobile, a lot architectures for embedded systems can be neglected. According to the [Steam hardware survey](https://web.archive.org/web/20210602040435/https://store.steampowered.com/hwsurvey/Steam-Hardware-Software-Survey-Welcome-to-Steam)
all users of Steam use hardware based on the x86 or x64 architecture. Since 99.75% of users run 64bit operating systems, and 32bit
systems have steadily declined in popularity, it can also be safe to assume that support for x64 is enough to cover the vast
majority of users on desktop. In regards to consoles, the [past few generations](https://en.wikipedia.org/wiki/Home_video_game_console#List_of_home_video_game_consoles)
have been using PowerPC (Wii U, Wii, XBox 360, Playstation 3 & 2, GameCube), x64 (Playstation 5 & 4, XBox Series X/S & One),
ARMv8 (Switch, Oculus Quest 1 & 2) and x86 (Xbox), with the facing out of both PowerPC and x86. Support for x64 and 64bit ARM will
therefore cover the newest two generations (except support for the Wii U), and seems to align with the direction of the console
industry. Mobile is not a concern for this programming language, and will therefore be neglected in its entirety.

The Niob programming language will therefore be designed to be maximally performant, reliable and true to hardware on x64 and
ARM with AArch64 support. This means the language will often be able to support other architectures, but they will have to
emulate the behaviour decided for x64 and ARM.

### Dealing with differences
To support more than one architecture it is often necessary to define some sort of abstract machine. It need not be as far
removed from hardware as e.g. the one in Forth, but any minute difference between architectures needs to either be solved by
providing a compromise (i.e. defining an abstract machine with a mapping to hardware), or choosing the behaviour on one
architecture or the other (essentially defining an abstract machine with the same semantics as the target architecture).
On modern architectures this is rather easy, since most machines use 2's complement little endian integers ans support
IEEE 754 floating point, however they do often differ when it comes to hardware exceptions and the effects of illegal
operations. An example of this is integer division by zero, which causes a divide error exception on x64, but no
exception on arm. There are however some operations that seem to be legal on x64, but not arm (e.g. bitwise shifts seem
to support negative shift values and interpret them as unsigned integers).

#### Integer representation and operations
##### x64
Integers on x64 are 2's complement signed and unsigned with sizes being powers of 2 ranging from 8 - 64 bit.
A special value is defined for each size, named the integer indefinite. It is equal to 2^(size - 1). This is
a special value returned by the x87 FPU when an invalid operation is detected when storing an interger in
memory with the FIST/FISTP instruction, and the invalid operation exception is masked.
Integers are represented in memory as consecutive bytes that are little endian.
Addition and subtraction zero/sign extends and truncates to largest operand size (effectively wrapping addition/subtraction).
Division zero/sign extends and truncates toward 0, hardware exception is thrown if divisor is 0 or the quotient is not representable as a 2's
complement integer with a size equal to the largest sized operand.
Multiplication zero/sign extends and truncates to largest operand size.

##### ARM
Integers on ARM are 2's complement signed and unsigned with sizes being powers of 2 ranging from 8 - 64 bit.
Integers are respresented in memory as consecutive bytes that are little endian.
Addition and subtraction zero/sign extends and truncates to largest operand size (effectively wrapping addition/subtraction)
Division zero/sign extends and truncates toward 0, 0 is returned if divisor is 0 and result is truncated to largest operand size.
Multiplication zero/sign extends and truncates to largest operand size.

##### Niob
Integers in Niob are 2's complement signed and unsigned with sizes being powers of 2 ranging from 8 - 64bit.
Integers are represented in memory as consecutive bytes that are little endian.
Addition, subtraction, division and multiplication all zero/sign extend and truncate result to largest operand size.
Division truncates toward 0.
Division by 0 and where the result is out of range (max and min value of largest operand size) are implementation defined.

##### Implications
Niob cannot guarantee the result of a divide by 0 and INT_MIN / -1

#### Floating point representation and operations
##### x64
x64 defines three floating point types: single-, double- and double extended precision which correspond to formats
specified in IEEE 754. Conversion to and from half precision floats is also possible. The single- and double
precision formats encode only the fractional part of the significand, as apposed to the double extended precision
format which stores the leading digit explicitly (bit 63). Half precision, single precision, double precision and
double extended precision are stored as 2, 4, 8 and 10 consecutive little endian bytes respectively. The double
extended precision format is only operatable on by the x87 FPU.
All operations obey IEEE 754.
Addition and subtraction where one of the operands are a signaling NaN, operands are infinities of (add: different, sub: like)
signs, source operand is a denormal value, result is too small, result is too large or the value cannot be represented exactly
causes a floating point exception.
Division where one of the operands is a signaling NaN, +- inf / +- inf, +- 0 / +- 0, source operand is a denormal value,
divisor is +- 0, result is too small, result is too large or the value cannot be represented exactly causes a floating point
exception.
Multiplication where one of the operands are a signaling NaN, one operand is +- 0 and the other is +- inf, source operand
is a denormal value, result is too small, result is too large or the value cannot be represented exactly causes a floating
point exception.
Conversion to integer rounds acording to the current rounding mode, floating point invalid exception is raised when the value
cannot be represented in the target type after rounding. If this exception is masked the indefinite integer value is returned.
Conversion from integer rounds according to the current rounding mode. An exception is raised when the result is not exactly
representable in the target type.
Conversion from double precision to single precision round according to the current rounding mode. An exception is raised when
the result is not exactly representable in the target type.

##### ARM
ARM defines three floating point types: half-, single- and double precision which correspond to formats specified in IEEE 754.
Conversion to and from a separate 16bit storage format is also possible. The half-, single- and double precision formats encode
only the fractional part of the significand, and are stored as 2, 4, and 8 consecutive little endian bytes respectively.
All operations obey IEEE 754.
The support for floating point exceptions is implementation defined.
Conversion to integer round toward 0.
Conversion from integer round to nearest floating point number.
Conversion between floating point types rounds to nearest.

##### Niob
Floating point number in Niob obey IEEE 754 and come in the sizes 32bit and 64bit. All operations obey IEEE 754, conversion to
integer rounds toward 0, conversion from integer rounds toward nearest. Conversion between floating point types roun toward nearest.
Exceptions thrown during invalid floating point operations is implementation defined.

##### Implications
Niob cannot guarantee anything about floating point exceptions.

#### Memory model
On x64 ...

On ARM ...

Rep, ops ...

Implications

Integers
Integers are 2's complement
Addition and subtraction are wrapping
Multiplication and division are masking
Shifts mask to legal range
Bitwise and, or, xor and not behave as expected
INT_MIN / -1, N / 0, conversions from OoB floats and negative shifts are implementation defined

Floating point
Floats are IEEE 754 and behave according to that standard
Exceptions may not be supported on ARM

## Syntax

## Values and types

## Expressions

## Statements

## ...
