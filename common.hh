#ifndef EVERLOT_COMMON_HH_
#define EVERLOT_COMMON_HH_ 1

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

// arch settings
#ifdef ARCH_X64
#	define ARCH_UNALIGNED_ACCESS 1
#	define ARCH_BIG_ENDIAN 0
#else
#	error "add arch settings"
#endif

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
#ifndef BUILD_DEBUG
#	define DEBUG_LOG(...)		((void)0)
#	define DEBUG_ASSERT(...)	((void)0)
#else
#	define DEBUG_LOG(...)		LOG(__VA_ARGS__)
#	define DEBUG_ASSERT(expr)	ASSERT(expr)
#endif

// compiler settings
#if defined(_MSC_VER)
#	define INLINE __forceinline
#	define UNREACHABLE abort() //do { __assume(0); abort(); } while(0)
#	define FALLTHROUGH ((void)0)
#elif defined(__GNUC__)
#	define INLINE __attribute__((always_inline)) inline
#	define UNREACHABLE abort() //do { __builtin_unreachable(); abort(); } while(0)
#	define FALLTHROUGH __attribute__((fallthrough))
#else
#	error "add compiler settings"
#endif

// common macros
#define EV_NARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
#define EV_IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

// TODO: logging
#include <stdio.h>
#define LOG(...)		fprintf(stdout, __FUNCTION__ ": " __VA_ARGS__)
#define LOG_ERROR(...)	fprintf(stdout, __FUNCTION__ ": " __VA_ARGS__)

#endif //EVERLOT_COMMON_HH_
