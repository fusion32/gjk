
#ifndef GL_PROC
#	define GL_PROC(rettype, name, args)
#endif

#ifndef GL_ENUM
#	define GL_ENUM(name, value)
#endif

// debug output
GL_ENUM(DEBUG_OUTPUT, 0x92E0)
GL_PROC(void, DebugMessageCallback, (GLDEBUGPROC callback, const void *userParam))

// fixed function state
GL_ENUM(FALSE, 0)
GL_ENUM(TRUE, 1)
GL_ENUM(DEPTH_BUFFER_BIT, 0x00000100)
GL_ENUM(COLOR_BUFFER_BIT, 0x00004000)
GL_ENUM(DEPTH_TEST, 0x0B71)
GL_PROC(void, Clear, (GLbitfield mask))
GL_PROC(void, ClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
GL_PROC(void, Enable, (GLenum cap))

// programmable state
GL_ENUM(FRAGMENT_SHADER, 0x8B30)
GL_ENUM(VERTEX_SHADER, 0x8B31)
GL_ENUM(COMPILE_STATUS, 0x8B81)
GL_ENUM(LINK_STATUS, 0x8B82)
GL_PROC(GLuint, CreateShader, (GLenum type))
GL_PROC(void, DeleteShader, (GLuint shader))
GL_PROC(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length))
GL_PROC(void, CompileShader, (GLuint shader))
GL_PROC(void, GetShaderiv, (GLuint shader, GLenum pname, GLint *params))
GL_PROC(void, GetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
GL_PROC(GLuint, CreateProgram, (void))
GL_PROC(void, DeleteProgram, (GLuint program))
GL_PROC(void, AttachShader, (GLuint program, GLuint shader))
GL_PROC(void, DetachShader, (GLuint program, GLuint shader))
GL_PROC(void, LinkProgram, (GLuint program))
GL_PROC(void, UseProgram, (GLuint program))
GL_PROC(void, GetProgramiv, (GLuint program, GLenum pname, GLint *params))
GL_PROC(void, GetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))

// draw calls
GL_ENUM(LINES, 0x0001)
GL_PROC(void, DrawArrays, (GLenum mode, GLint first, GLsizei count))

// vertex attributes
GL_ENUM(FLOAT, 0x1406)
GL_PROC(void, GenVertexArrays, (GLsizei n, GLuint *arrays))
GL_PROC(void, BindVertexArray, (GLuint array))
GL_PROC(void, DisableVertexAttribArray, (GLuint index))
GL_PROC(void, EnableVertexAttribArray, (GLuint index))
GL_PROC(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))

// buffer
GL_ENUM(STATIC_DRAW, 0x88E4)
GL_ENUM(ARRAY_BUFFER, 0x8892)
GL_ENUM(UNIFORM_BUFFER, 0x8A11)
GL_PROC(void, GenBuffers, (GLsizei n, GLuint *buffers))
GL_PROC(void, BindBuffer, (GLenum target, GLuint buffer))
GL_PROC(void, BufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
GL_PROC(void, BindBufferBase, (GLenum target, GLuint index, GLuint buffer))

// get string
GL_ENUM(VENDOR, 0x1F00)
GL_ENUM(RENDERER, 0x1F01)
GL_ENUM(VERSION, 0x1F02)
GL_ENUM(EXTENSIONS, 0x1F03)
GL_ENUM(SHADING_LANGUAGE_VERSION, 0x8B8C)
GL_PROC(const GLubyte*, GetString, (GLenum name))
GL_PROC(const GLubyte*, GetStringi, (GLenum name, GLuint index))


#undef GL_ENUM
#undef GL_PROC
