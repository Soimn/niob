# Niob Language Specification

The Niob programming language is a procedural systems programming language that aims to replace C for game programming.

## The problem with C
There are countless problems with C in regards to the abstracted view of programming language design. Although this language
tries to resolve those problems, they are not the primary concern. What the Niob programming language aims to do is to resolve
all undefined behaviour introduced in C. A significant portion of the undefined behaviour, listed in appendix J of the
ISO/IEC 9899:201x n1570 publicly available draft, of the C11 standard can easily be dealt with by throwing away the standard library
and removing the notion of translation units. What is left is a small list of undefined behaviour that concerns integer and floating
point operations, hardware exceptions, calling conventions, evaluation order, unaligned memory access, calling functions by pointer
with wrong arguments, modification of the data segment. There are however a lot of undefined behaviour that is not listed in this
appendix (e.g. infinite loops without side effects are undefined behaviour).

## Resolving the undefined behaviour
The approach chosen to resolve the undefined behaviour is to first restrict the problem to only relevant architectures, broker between
these architectures to find a common representation that is as close to the actual hardware as possible and then dealing with ways of
expressing architecture dependent behaviour.

### Restricting platform support
Different architectures implement arithmetic operations and exceptions differently, and may also use a different memory model. To broker
between every architecture in existence will quickly lead to the mess that C has made for itself. Since games primarily target desktop,
console and mobile, a lot architectures for embedded systems can be neglected. According to the Steam hardware survey
(https://store.steampowered.com/hwsurvey/Steam-Hardware-Software-Survey-Welcome-to-Steam)
all users of Steam use hardware based on the x86 or x64 architecture. Since 99.75% of users run 64bit operating systems, and 32bit
systems have steadily declined in popularity, it can also be safe to assume that support for x64 is enough to cover the vast
majority of users on desktop. In regards to consoles, the past few generations have been using
PowerPC (Wii U, Wii, XBox 360, Playstation 3 & 2, GameCube), x64 (Playstation 5 & 4, XBox Series X/S & One),
ARMv8 (Switch, Oculus Quest 1 & 2) and x86 (Xbox), with the facing out of both PowerPC and
x86 (https://en.wikipedia.org/wiki/Home_video_game_console#List_of_home_video_game_consoles). Support for x64 and 64bit ARM will
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
On modern architectures this is rather easy, since most machines are little endian with IEEE 754 floating point support,
however they do often differ when it comes to hardware exceptions and the effects of illegal operations.

## Syntax

## Integers
Integers are 2's complement
Addition and subtraction are wrapping
Multiplication and division are masking
Shifts mask to legal range
Bitwise and, or, xor and not behave as expected
INT_MIN / -1, N / 0, conversions from OoB floats and negative shifts are implementation defined

## Floating point
Floats are IEEE 754 and behave according to that standard
Exceptions may not be supported on ARM
