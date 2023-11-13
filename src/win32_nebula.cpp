#include <math.h>
#include <stdint.h>

#define global static
#define pi32 3.14159265359f
#define KEEB_COUNT 1

#include <Windows.h>
#include <stdio.h>
#include <dsound.h>
#include <gl/GL.h>

#include "nebula.h"

// NOTE: Opengl definitions
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

#include "win32_nebula.h"

global b32 g_running = false;
global win32_bitmap_buffer g_bm_buffer;
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
global thread_context g_thread_context = {};
global i64 g_perf_count_freq;
global GLuint g_shader_program;

#include "nebula.cpp"

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

static GLuint create_ogl_shader_program(char *vertex_file_name, char *fragment_file_name)
{
    GLuint prog_id = 0;
    debug_file_result s_vertex_file = DEBUG_read_entire_file(&g_thread_context, vertex_file_name);
    debug_file_result s_fragment_file = DEBUG_read_entire_file(&g_thread_context, fragment_file_name);

    // Create shaders
    const char *vertexShaderSource = (char *)s_vertex_file.contents;
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex_shader);
    // Check the shader was compiled successfully
    i32 success;
    char info[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char put_string[512];
        glGetShaderInfoLog(vertex_shader, arr_count(info), NULL, info);
        _snprintf_s(put_string, sizeof(put_string), "Failed vertex shader compilation: %s\n",
                    info);
        OutputDebugStringA(put_string);
    }
    const char *fragmentShaderSource = (char *)s_fragment_file.contents;
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char put_string[512];
        glGetShaderInfoLog(fragment_shader, arr_count(info), NULL, info);
        _snprintf_s(put_string, sizeof(put_string), "Failed fragment shader compilation: %s\n",
                    info);
        OutputDebugStringA(put_string);
    }
    // Create shader program
    prog_id = glCreateProgram();
    glAttachShader(prog_id, vertex_shader);
    glAttachShader(prog_id, fragment_shader);
    glLinkProgram(prog_id);
    glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        char put_string[512];
        glGetProgramInfoLog(prog_id, arr_count(info), NULL, info);
        _snprintf_s(put_string, sizeof(put_string), "Failed shader program link: %s\n",
                    info);
        OutputDebugStringA(put_string);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    DEBUG_free_file(&g_thread_context, s_vertex_file.contents);
    DEBUG_free_file(&g_thread_context, s_fragment_file.contents);

    return prog_id;
}

static void win32_init_opengl(HWND window_handle)
{
    HDC window_dc = GetDC(window_handle);

    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    int given_pixelformat_index = ChoosePixelFormat(window_dc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR given_pixelformat;
    DescribePixelFormat(window_dc, given_pixelformat_index, sizeof(given_pixelformat), &given_pixelformat);
    SetPixelFormat(window_dc, given_pixelformat_index, &given_pixelformat);

    HGLRC opengl_rc = wglCreateContext(window_dc);
    if(!wglMakeCurrent(window_dc, opengl_rc))
    {
        // TODO: Diags
        invalid_code_path;
    }

    // OpenGL func ptrs
    glGenBuffers = (glgenbuffers *)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (glbindbuffer *)wglGetProcAddress("glBindBuffer");
    glBufferData = (glbufferdata *)wglGetProcAddress("glBufferData");
    glCreateShader = (glcreateshader *)wglGetProcAddress("glCreateShader");
    glShaderSource = (glshadersource *)wglGetProcAddress("glShaderSource");
    glCompileShader = (glcompileshader *)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (glgetshaderiv *)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (glgetshaderinfolog *)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (glcreateprogram *)wglGetProcAddress("glCreateProgram");
    glAttachShader = (glattachshader *)wglGetProcAddress("glAttachShader");
    glLinkProgram = (gllinkprogram *)wglGetProcAddress("glLinkProgram");
    glUseProgram = (gluseprogram *)wglGetProcAddress("glUseProgram");
    glDeleteShader = (gldeleteshader *)wglGetProcAddress("glDeleteShader");
    glGetProgramiv = (glgetprogramiv *)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (glgetprograminfolog *)wglGetProcAddress("glGetProgramInfoLog");
    glVertexAttribPointer = (glvertexattribpointer *)wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (glenablevertexattribarray *)wglGetProcAddress("glEnableVertexAttribArray");
    glGenVertexArrays = (glgenvertexarrays *)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (glbindvertexarray *)wglGetProcAddress("glBindVertexArray");
    glGetUniformLocation = (glgetuniformlocation *)wglGetProcAddress("glGetUniformLocation");
    glUniform4f = (gluniform4f *)wglGetProcAddress("glUniform4f");
    glGenerateMipmap = (glgeneratemipmap *)wglGetProcAddress("glGenerateMipmap");
    glActiveTexture = (glactivetexture *)wglGetProcAddress("glActiveTexture");
    glUniform1i = (gluniform1i *)wglGetProcAddress("glUniform1i");

    // Create Shader Program
    g_shader_program = create_ogl_shader_program("..\\vec.glsl", "..\\frag.glsl");
    // Vertex Data
    // v3 vertices[] = {
    //     { -0.5f, -0.5f, 0.0f }, // V1 pos data
    //     { 1.0f, 0.0f, 0.0f }, // V1 color data
    //     { 0.5f, -0.5f, 0.0f }, // V2 pos data
    //     { 0.0f, 1.0f, 0.0f }, // V2 color data
    //     { 0.0f, 0.5f, 0.0f }, // V3 pos data
    //     { 0.0f, 0.0f, 1.0f }, // V3 color data
    // };
    //
    // v2 tex_coords[] = {
    //     { 0.0f, 0.0f },
    //     { 1.0f, 0.0f },
    //     { 0.5f, 1.0f },
    // };

    // v3 vertices[] = {
    //     { 0.5f, 0.5f, 0.0f },   // top-right
    //     { 0.5f, -0.5f, 0.0f },  // bottom-right
    //     { -0.5f, -0.5f, 0.0f }, // bottom-left
    //     { -0.5f, 0.5f, 0.0f },  // top-left
    // };
    // TODO: Need to move the data for quads to
    // build the flow for working with a texture atlas
    f32 vertices[] = {
        0.75f, 0.75f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // top-right
        0.75f, -0.75f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        -0.75f, -0.75f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        -0.75f, 0.75f, 0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f   // top-left
    };
    u32 indices[] = {
        0, 1, 3,    // T1
        1, 2, 3     // T2
    };

    // Create a (V)ertex (B)uffer (O)bject and (V)ertex (A)rray (O)bject
    u32 VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VAO, the bind and set VBOs, then config vertex attribs
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell opengl how to interpret our vertex data by setting pointers to the attribs
    // pos attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    // color attrib
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(3*sizeof(f32)));
    glEnableVertexAttribArray(1);
    // tex coord attrib
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(6*sizeof(f32)));
    glEnableVertexAttribArray(2);

    ReleaseDC(window_handle, window_dc);
}

static win32_win_dimensions win32_get_win_dimensions(HWND win_handle)
{
  win32_win_dimensions window_dimensions = {};
  RECT client_rect;
  GetClientRect(win_handle, &client_rect);

  window_dimensions.width = client_rect.right - client_rect.left;
  window_dimensions.height = client_rect.bottom - client_rect.top;

  return window_dimensions;
}

static void win32_resize_DIB_section(win32_bitmap_buffer *buffer, int _width,
                                     int _height)
{
  // Need to free the memory but only when it will be reallocated right away
  // since a paint will happen every frame
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = _width;
    buffer->height = _height;

    // Step one is to create the DIB
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = _width;
    buffer->info.bmiHeader.biHeight = -_height; // Making this negative to have the bitmap layout start top-left and go top-down
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->bytes_per_pixel = 4;
    int bm_mem_size = buffer->bytes_per_pixel * (_width * _height);
    buffer->memory =
        VirtualAlloc(0, bm_mem_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = buffer->bytes_per_pixel * _width;
}

inline LARGE_INTEGER win32_get_seconds_wallclock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline f32 win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = ((f32) (end.QuadPart - start.QuadPart) / (f32) g_perf_count_freq);
    return result;
}

// This is needed to force rePaint to get a set fps
static void win32_update_win_with_buffer(HDC device_context,
                                         engine_bitmap_buffer *buffer,
                                         int win_width, int win_height)
{
#if 0
    // NOTE: Need to make a win32_bitmap_buffer when using this bliting

    // Clear the unused client pixels to black
    PatBlt(device_context, 0, g_bm_buffer.height, win_width, win_height, BLACKNESS);
    PatBlt(device_context, g_bm_buffer.width, 0, win_width, win_height, BLACKNESS);

    // TODO:
    // Fix the aspect ratio with math (either through floating point or
    // integer)
    StretchDIBits(
        device_context,
        // _x, _y, _width, _height, // Props for dest bitmap
        // _x, _y, _width, _height, // Props of src bitmap
        // To make a simple rect first, we  stretch the bits to the whole window
        // NOTE: HHD[024]: casey wanted to make the displayed pixels be one-to-one
        // so instead of taking in the win dimensions for this function,
        // we will be making it based on the g_bm_buffer's initial dims
        0, 0, g_bm_buffer.width, g_bm_buffer.height,
        0, 0, g_bm_buffer.width, g_bm_buffer.height,
        g_bm_buffer.memory, &g_bm_buffer.info,
        DIB_RGB_COLORS, // iUsage - Use literal rgb values to color in the pixels
        SRCCOPY);       // Raster Operation Code - Copy our bitmap to the dest

#else
    glViewport(0, 0, win_width, win_height);
    glClearColor(0.8f, 0.56f, 0.64f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // DRAW
    // glPolygonMode(GL_FRONT, GL_LINE);
    glUseProgram(g_shader_program);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // TODO: Look up what swapbuffers should be used
    SwapBuffers(device_context);
#endif
}
static LRESULT CALLBACK win32_main_window_callback(HWND win_handle,
                                                   UINT message, WPARAM WParam,
                                                   LPARAM LParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            g_running = false;
            break;
        }

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            g_running = false;
            break;
        }

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            neo_assert(!"NO INPUT HERE");
        }

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
            break;
        }

        default:
        {
            result = DefWindowProc(win_handle, message, WParam, LParam);
        }
    }

    return result;
}

static void win32_process_keeb_message(engine_button_state *new_state, b32 is_down)
{
    if (new_state->is_down != is_down)
    {
        new_state->is_down = is_down;
        ++new_state->half_transitions;
    }
}

static void win32_process_pending_win_messages(engine_controller_input *keyboard)
{
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                // virt keycode that tells us what the key was
                // Okay to truncate b/c vk_codes are not 64-bit![HHD016]
                u32 vk_code = (u32) message.wParam;
                b32 was_down = ((message.lParam & (1 << 30)) != 0);
                b32 is_down = ((message.lParam & (1 << 31)) == 0);
                if (is_down != was_down)
                {
                    switch (vk_code)
                    {
                        case 'W':
                        {
                            win32_process_keeb_message(&keyboard->move_up, is_down);
                        } break;
                        case 'A':
                        {
                            win32_process_keeb_message(&keyboard->move_left, is_down);
                        } break;
                        case 'S':
                        {
                            win32_process_keeb_message(&keyboard->move_down, is_down);
                        } break;
                        case 'D':
                        {
                            win32_process_keeb_message(&keyboard->move_right, is_down);
                        } break;
                        case 'Q':
                        {
                            win32_process_keeb_message(&keyboard->left_shoulder, is_down);
                        } break;
                        case 'E':
                        {
                            win32_process_keeb_message(&keyboard->right_shoulder, is_down);
                        } break;
                        case VK_UP:
                        {
                            win32_process_keeb_message(&keyboard->action_up, is_down);
                        } break;
                        case VK_DOWN:
                        {
                            win32_process_keeb_message(&keyboard->action_down, is_down);
                        } break;
                        case VK_LEFT:
                        {
                            win32_process_keeb_message(&keyboard->action_left, is_down);
                        } break;
                        case VK_RIGHT:
                        {
                            win32_process_keeb_message(&keyboard->action_right, is_down);
                        } break;
                        case VK_SPACE:
                        {
                            // char put_string[256];
                            //
                            // new_hand_deal(&base_deck);
                            // _snprintf_s(put_string, sizeof(put_string), "RANK: %d SUIT: %s\n",
                            //             base_deck.cards[4].value, base_deck.cards[4].suit);
                            // OutputDebugStringA(put_string);
                        } break;
                        case VK_ESCAPE:
                        {
                            win32_process_keeb_message(&keyboard->start, is_down);
                            // TODO: this really needs to be sent to our main menu
                            g_running = false;
                        } break;
                        case VK_BACK:
                        {
                            win32_process_keeb_message(&keyboard->back, is_down);
                        } break;
                    }
                }

                b32 alt_key_was_down = ((message.lParam & (1 << 29)) != 0);
                if ((vk_code == VK_F4) && alt_key_was_down)
                {
                  g_running = false;
                }
            } break;

            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;

        }

     }
}

INT WINAPI WinMain(HINSTANCE win_instance, HINSTANCE prev_instance,
                   PSTR cmd_line, INT show_command)
{
    UINT desired_scheduler_timing = 1;
    b32 granular_sleep = (timeBeginPeriod(desired_scheduler_timing) == TIMERR_NOERROR);

    LARGE_INTEGER perf_count_freq;
    QueryPerformanceFrequency(&perf_count_freq);
    g_perf_count_freq = perf_count_freq.QuadPart;

    // Creates a window class that defines a window
    WNDCLASSA win_class = {};
    win_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // necessary flags to redraw the entire window.
    win_class.lpfnWndProc = win32_main_window_callback;
    win_class.hInstance = win_instance;
    win_class.lpszClassName = "MainWindowClass";

    win32_resize_DIB_section(&g_bm_buffer, 960, 540);

    if (!RegisterClassA(&win_class))
        return false;

    HWND window = CreateWindowExA(0, win_class.lpszClassName, "EngineNeo",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0,
                                0, win_instance, 0);
    if(!window)
        return false;

    // Based on how the device context system functions, when specifying a
    // CS_OWNDC flag for the window, it is possible to get the DC for the window
    // and then never give it back because we now own it for the duration of the
    // program.
    HDC device_context = GetDC(window);

    // NOTE: Due to having a higher than 60hz display, I will figure out the GetDeviceCaps call later!
    int monitor_hz = 60;

    f32 update_hz = monitor_hz / 2.0f;
    f32 target_seconds_per_frame = 1.0f / update_hz;

#if NEO_INTERNAL
    LPVOID base_address = (LPVOID) terabytes(2);
#else
    LPVOID base_address = 0;
#endif

    app_memory app_memory = {};
    app_memory.perm_storage_space = megabytes(64);
    app_memory.flex_storage_space = gigabytes(4);
    app_memory.DEBUG_free_file = DEBUG_free_file;
    app_memory.DEBUG_read_entire_file = DEBUG_read_entire_file;
    app_memory.DEBUG_write_entire_file = DEBUG_write_entire_file;

    // TODO: Probably want to look into MEM_LARGE_PAGES?? HHD[024]
    u64 total_size = app_memory.perm_storage_space + app_memory.flex_storage_space;
    app_memory.perm_mem_storage =
        VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    app_memory.flex_mem_storage =
        ((u8 *) app_memory.perm_mem_storage + app_memory.perm_storage_space);

#if 0
    loaded_jpg crate_tex = {};
    crate_tex = DEBUG_load_jpg(&global_arena, &g_thread_context, DEBUG_read_entire_file, "test/cardback.jpeg", DEBUG_free_file);
    if (crate_tex.pixels)
    {
        // Opengl
        win32_init_opengl(window, crate_tex);
    }
#endif

    // Init OpenGL
    win32_init_opengl(window);

    engine_input input[2] = {};
    engine_input *new_input = &input[0];
    engine_input *old_input = &input[1];

    // Global Loop
    g_running = true;
    while (g_running)
    {
        new_input->time_step_over_frame = target_seconds_per_frame;

        engine_controller_input *new_keeb = get_controller(new_input, 0);
        engine_controller_input *old_keeb = get_controller(old_input, 0);
        *new_keeb = {};

        for (int button_index = 0;
             button_index < arr_count(new_keeb->buttons);
             button_index++)
        {
            new_keeb->buttons[button_index].is_down = old_keeb->buttons[button_index].is_down;
        }

        win32_process_pending_win_messages(new_keeb);

        POINT mouse_point;
        GetCursorPos(&mouse_point);
        ScreenToClient(window, &mouse_point);
        new_input->mouseX = mouse_point.x;
        new_input->mouseY = mouse_point.y;
        new_input->mouseZ = 0;
        win32_process_keeb_message(&new_input->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
        win32_process_keeb_message(&new_input->mouse_buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
        win32_process_keeb_message(&new_input->mouse_buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));

        DWORD max_controller_count = KEEB_COUNT;
        if (max_controller_count > arr_count(new_input->controllers)) {
          max_controller_count = arr_count(new_input->controllers);
        }
        // First controller is the keyboard
        for (DWORD controller_index = KEEB_COUNT; controller_index < max_controller_count;
             controller_index++)
        {

            engine_controller_input *old_control_state =
                get_controller(old_input, controller_index);
            engine_controller_input *new_control_state =
                get_controller(new_input, controller_index);

        }

        thread_context thread = {};
        // This pulls from our platform-independent code from nebula.h
        engine_bitmap_buffer bm_buffer = {};
        bm_buffer.memory = g_bm_buffer.memory;
        bm_buffer.width = g_bm_buffer.width;
        bm_buffer.height = g_bm_buffer.height;
        bm_buffer.pitch = g_bm_buffer.pitch;
        bm_buffer.bytes_per_pixel = g_bm_buffer.bytes_per_pixel;
        update_and_render(&thread, &app_memory, new_input, &bm_buffer);

        win32_win_dimensions win_size = win32_get_win_dimensions(window);
        win32_update_win_with_buffer(device_context, &bm_buffer,
                                     win_size.width, win_size.height);

        engine_input *temp = new_input;
        new_input = old_input;
        old_input = temp;
    }

    return 0;
}
