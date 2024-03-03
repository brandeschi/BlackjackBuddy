// NOTE: WIN32 Opengl definitions
typedef i64 GLsizeiptr;
typedef char GLchar;
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_MIRRORED_REPEAT                0x8370
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE0                       0x84C0
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

// Accepted as an attribute name in <*attribList>:
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

// Accepted as bits in the attribute value for WGL_CONTEXT_FLAGS in
// <*attribList>:
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// Accepted as bits in the attribute value for
// WGL_CONTEXT_PROFILE_MASK_ARB in <*attribList>:
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef void glgenbuffers(GLsizei n, GLuint *buffers);
global glgenbuffers *glGenBuffers;
typedef void glbindbuffer(GLenum target, GLuint buffer);
global glbindbuffer *glBindBuffer;
typedef void glbufferdata(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
global glbufferdata *glBufferData;
typedef GLuint glcreateshader(GLenum type);
global glcreateshader *glCreateShader;
typedef void glshadersource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
global glshadersource *glShaderSource;
typedef void glcompileshader(GLuint shader);
global glcompileshader *glCompileShader;
typedef void glgetshaderiv(GLuint shader, GLenum pname, GLint *params);
global glgetshaderiv *glGetShaderiv;
typedef void glgetshaderinfolog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
global glgetshaderinfolog *glGetShaderInfoLog;
typedef GLuint glcreateprogram(void);
global glcreateprogram *glCreateProgram;
typedef void glattachshader(GLuint program, GLuint shader);
global glattachshader *glAttachShader;
typedef void gllinkprogram(GLuint program);
global gllinkprogram *glLinkProgram;
typedef void gluseprogram(GLuint program);
global gluseprogram *glUseProgram;
typedef void gldeleteshader(GLuint shader);
global gldeleteshader *glDeleteShader;
typedef void glgetprogramiv(GLuint program, GLenum pname, GLint *params);
global glgetprogramiv *glGetProgramiv;
typedef void glgetprograminfolog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
global glgetprograminfolog *glGetProgramInfoLog;
typedef void glvertexattribpointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
global glvertexattribpointer *glVertexAttribPointer;
typedef void glenablevertexattribarray(GLuint index);
global glenablevertexattribarray *glEnableVertexAttribArray;
typedef void glgenvertexarrays(GLsizei n, GLuint *arrays);
global glgenvertexarrays *glGenVertexArrays;
typedef void glbindvertexarray(GLuint array);
global glbindvertexarray *glBindVertexArray;
typedef GLint glgetuniformlocation(GLuint program, const GLchar *name);
global glgetuniformlocation *glGetUniformLocation;
typedef void gluniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
global gluniform4f *glUniform4f;
typedef void glgeneratemipmap(GLenum target);
global glgeneratemipmap *glGenerateMipmap;
typedef void glactivetexture(GLenum texture);
global glactivetexture *glActiveTexture;
typedef void gluniform1i(GLint location, GLint v0);
global gluniform1i *glUniform1i;
typedef void gluniformmatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
global gluniformmatrix4fv *glUniformMatrix4fv;

struct entity_render_data
{
    int x;
};

struct vertex_data {
    v3 position;
    v3 color;
    v2 tex_coords;
};
