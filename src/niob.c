#include "niob.h"

#define ASSERT(EX) ((EX) ? 1 : *(volatile int*)0)

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")
#define INVALID_DEFAULT_CASE ASSERT(!"INVALID_DEFAULT_CASE")
#define INVALID_CODE_PATH ASSERT(!"INVALID_CODE_PATH")

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define OFFSETOF(T, e) (umm)&((T*)0)->e
#define ALIGNOF(T) (u8)OFFSETOF(struct { char c; T t; }, t)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define STR(S) (String){.data = (u8*)(S), .size = sizeof(S) - 1}

#define U8_MAX  (u8) 0xFF
#define U16_MAX (u16)0xFFFF
#define U32_MAX (u32)0xFFFFFFFF
#define U64_MAX (u64)0xFFFFFFFFFFFFFFFF

#define I8_MAX  (i8) U8_MAX   >> 1
#define I16_MAX (i16)U16_MAX >> 1
#define I32_MAX (i32)U32_MAX >> 1
#define I64_MAX (i64)U64_MAX >> 1

#define I8_MIN  (i8) (1 << 7)
#define I16_MIN (i16)(1 << 15)
#define I32_MIN (i32)(1 << 31)
#define I64_MIN (i64)(1 << 63)

#include <wmmintrin.h>

#include "memory.h"

#include "lexer.h"
#include "parser.h"