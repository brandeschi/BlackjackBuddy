struct win32_bitmap_buffer
{
    BITMAPINFO info; // BITMAPINFO bmi; this is a struct
    void *memory;
    int pitch;
    int bytes_per_pixel;
    int width;
    int height;
};

struct win32_win_dimensions
{
    int width;
    int height;
};

struct win32_sound_settings
{
    int samples_per_sec;
    u32 running_sample_index;
    int bytes_per_sample;
    DWORD secondary_buffer_size;
    DWORD cushion_bytes;
};

// NOTE: Debug file handling
DEBUG_FREE_FILE_MEMORY(DEBUG_free_file)
{
    if (file)
    {
        VirtualFree(file, 0, MEM_RELEASE);
    }
}

DEBUG_READ_ENTIRE_FILE(DEBUG_read_entire_file)
{
    debug_file_result result = {};
    HANDLE file_handle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size))
        {
           u32 file_size32 = safe_truncate_int64(file_size.QuadPart);
           result.contents = VirtualAlloc(0, file_size32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
           if (result.contents)
           {
               DWORD bytes_read;
               if (ReadFile(file_handle, result.contents, file_size32, &bytes_read, 0) &&
                   (file_size32 == bytes_read))
               {
                   result.contents_size = file_size32;
               }
               else
               {
                   DEBUG_free_file(thread, result.contents);
                   result.contents = 0;
               }
           }
        }
    }
    CloseHandle(file_handle);
    return result;
}

DEBUG_WRITE_ENTIRE_FILE(DEBUG_write_entire_file)
{
    b32 result = false;
    HANDLE file_handle = CreateFile(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        DWORD bytes_written;
        if (WriteFile(file_handle, file, file_size, &bytes_written, 0))

        {
            result = (bytes_written == file_size);
        }
        else
        {
            // TODO: LOGGING
        }

    }

    CloseHandle(file_handle);
    return result;
}

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
#define GL_TEXTURE0                       0x84C0

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
