#ifndef GJK_COMMON_HH_
#define GJK_COMMON_HH_ 1

// ensure we're compiling in 64bit
static_assert(sizeof(void*) == 8, "");

// stdlib base
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// int types
typedef int8_t		i8;
typedef uint8_t		u8;
typedef int16_t		i16;
typedef uint16_t	u16;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef int64_t		i64;
typedef uint64_t	u64;
typedef size_t		usize;

// float types
typedef float		f32;
typedef double		f64;

// debug settings
#define ASSERT(expr)								\
	do{ if(!(expr)){								\
		LOG("%s:%d: \"%s\" assertion failed\n",		\
			__FILE__, __LINE__, #expr);				\
		abort();									\
	} } while(0)
#ifdef _DEBUG
#	define BUILD_DEBUG 1
#endif

// compiler settings
#if defined(_MSC_VER)
#	define INLINE __forceinline
#elif defined(__GNUC__)
#	define INLINE __attribute__((always_inline)) inline
#else
#	define INLINE inline
#endif

// common macros
#define NARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

// TODO: logging
#include <stdio.h>
#define LOG(...)		fprintf(stdout, __FUNCTION__ ": " __VA_ARGS__)
#define LOG_ERROR(...)	fprintf(stdout, __FUNCTION__ ": " __VA_ARGS__)

#endif //GJK_COMMON_HH_
