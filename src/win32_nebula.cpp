#include "win32.unity.h"

// typedef HGLRC WINAPI wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

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
  OutputDebugStringA((char *)glGetString(GL_VERSION));
  OutputDebugStringA("\n");

  wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
  if (wglCreateContextAttribsARB) {
    HGLRC shared_context = 0;
    int attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 5,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      // WGL_CONTEXT_DEBUG_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };
    HGLRC modern_glrc = wglCreateContextAttribsARB(window_dc, shared_context, attribs);
    if (modern_glrc) {
      if (wglMakeCurrent(window_dc, modern_glrc)) {
        wglDeleteContext(opengl_rc);
        opengl_rc = modern_glrc;
        OutputDebugStringA((char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
        OutputDebugStringA("\n");
      }
    }
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
  glUniformMatrix4fv = (gluniformmatrix4fv *)wglGetProcAddress("glUniformMatrix4fv");

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
  // TODO: Do aspect ratio
  //
  // glMatrixMode(GL_PROJECTION);
  // glLoadIdentity();
  // gluPerspective(45.0, 16.0/9.0*float(win_width)/float(win_height), 0.1, 100.0);

  glClearColor(0.2f, 0.66f, 0.44f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // DRAW
  // glPolygonMode(GL_FRONT, GL_LINE);
  glUseProgram(g_shader_program);
  // glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

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

  HWND window = CreateWindowExA(0, win_class.lpszClassName, "Blackjack Buddy",
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
