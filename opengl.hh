#ifndef GJK_OPENGL_HH_
#define GJK_OPENGL_HH_ 1

#include <stdint.h>

// NOTE: On WIN32 these are declared __stdcall. On WIN64, __stdcall
// was replaced by the win64 call convention so it can be ignored.
#ifndef APIENTRY
	#ifdef PLATFORM_WINDOWS
		#define APIENTRY __stdcall
	#else
		#define APIENTRY
	#endif
#endif

// GL Base Types
// 1.0
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
// 1.1
typedef float GLclampf;
typedef double GLclampd;
// 1.5
typedef intptr_t GLsizeiptr;
typedef intptr_t GLintptr;
// 2.0
typedef char GLchar;
typedef short GLshort;
typedef signed char GLbyte;
typedef unsigned short GLushort;
// 3.0
typedef unsigned short GLhalf;
// 3.2
typedef struct __GLsync *GLsync;
typedef uint64_t GLuint64;
typedef int64_t GLint64;
// 4.3
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

// GL Enums
enum{
	#define GL_ENUM(name, value) GL_##name = value,
	#include "opengl.inl"
};

// GL Function Types
#define GL_PROC(rettype, name, args) typedef rettype (APIENTRY *PFN_gl##name) args;
#include "opengl.inl"

// GL API:
//	Loading these function pointers is handled on the
// platform layer and if we get into `game_init` and
// `game_update_and_render` we can be certain they are
// already loaded.
//
//	OPENGL_DECLARE_FUNCTION_POINTERS should be declared
// before including this file on the platform layer main
// file.
//
#if OPENGL_DECLARE_FUNCTION_POINTERS
#	define GL_PROC(_1, name, _2) PFN_gl##name gl##name;
#else
#	define GL_PROC(_1, name, _2) extern PFN_gl##name gl##name;
#endif
#include "opengl.inl"

#endif //GJK_OPENGL_HH_
