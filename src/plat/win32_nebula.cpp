#pragma once
#include "../core.unity.h"
#include "win32_nebula.h"

// Globals
global b32 g_Running = false;
global win32_bitmap_buffer g_BitmapBuffer;
global LPDIRECTSOUNDBUFFER g_SecondaryBuffer;
global s64 g_PerfCountFreq;

// typedef HGLRC WINAPI wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

static void win32_InitOpengl(HWND WindowHandle)
{
  HDC WindowDC = GetDC(WindowHandle);

  PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
  DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
  DesiredPixelFormat.nVersion = 1;
  DesiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
  DesiredPixelFormat.cColorBits = 32;
  DesiredPixelFormat.cAlphaBits = 8;
  int GivenPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
  PIXELFORMATDESCRIPTOR GivenPixelFormat;
  DescribePixelFormat(WindowDC, GivenPixelFormatIndex, sizeof(GivenPixelFormat), &GivenPixelFormat);
  SetPixelFormat(WindowDC, GivenPixelFormatIndex, &GivenPixelFormat);

  HGLRC OpenglRC = wglCreateContext(WindowDC);
  if(!wglMakeCurrent(WindowDC, OpenglRC))
  {
    // TODO: Diags
    InvalidCodePath;
  }
  OutputDebugStringA((char *)glGetString(GL_VERSION));
  OutputDebugStringA("\n");

  wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
  if (wglCreateContextAttribsARB) {
    HGLRC SharedContext = 0;
    int Attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 5,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      // WGL_CONTEXT_DEBUG_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };
    HGLRC ModernGLRC = wglCreateContextAttribsARB(WindowDC, SharedContext, Attribs);
    if (ModernGLRC) {
      if (wglMakeCurrent(WindowDC, ModernGLRC)) {
        wglDeleteContext(OpenglRC);
        OpenglRC = ModernGLRC;
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

  ReleaseDC(WindowHandle, WindowDC);
}

static win32_win_dimensions win32_GetWindowDims(HWND WindowHandle)
{
  win32_win_dimensions WindowDims = {};
  RECT ClientRect;
  GetClientRect(WindowHandle, &ClientRect);

  WindowDims.width = ClientRect.right - ClientRect.left;
  WindowDims.height = ClientRect.bottom - ClientRect.top;

  return WindowDims;
}

static void win32_ResizeDIBSection(win32_bitmap_buffer *Buffer,
                                   int _Width, int _Height)
{
  // Need to free the memory but only when it will be reallocated right away
  // since a paint will happen every frame
  if (Buffer->memory)
  {
    VirtualFree(Buffer->memory, 0, MEM_RELEASE);
  }

  Buffer->width = _Width;
  Buffer->height = _Height;

  // Step one is to create the DIB
  Buffer->info.bmiHeader.biSize = sizeof(Buffer->info.bmiHeader);
  Buffer->info.bmiHeader.biWidth = _Width;
  Buffer->info.bmiHeader.biHeight = -_Height; // Making this negative to have the bitmap layout start top-left and go top-down
  Buffer->info.bmiHeader.biPlanes = 1;
  Buffer->info.bmiHeader.biBitCount = 32;
  Buffer->info.bmiHeader.biCompression = BI_RGB;

  Buffer->bytes_per_pixel = 4;
  int BitmapMemSize = Buffer->bytes_per_pixel * (_Width * _Height);
  Buffer->memory = VirtualAlloc(0, BitmapMemSize,
                                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  Buffer->pitch = Buffer->bytes_per_pixel * _Width;
}

inline LARGE_INTEGER win32_GetSecondsWallclock()
{
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return Result;
}

inline f32 win32_GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
  f32 Result = ((f32) (End.QuadPart - Start.QuadPart) / (f32) g_PerfCountFreq);
  return Result;
}

// This is needed to force rePaint to get a set fps
static void win32_UpdateWindownWithBuffer(HDC DeviceContext,
                                          engine_bitmap_buffer *Buffer,
                                          int WindowWidth, int WindowHeight)
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
  glViewport(0, 0, WindowWidth, WindowHeight);
  // TODO: Do aspect ratio
  //
  // glMatrixMode(GL_PROJECTION);
  // glLoadIdentity();
  // gluPerspective(45.0, 16.0/9.0*float(win_width)/float(win_height), 0.1, 100.0);

  glClearColor(0.2f, 0.66f, 0.44f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // DRAW
  // glPolygonMode(GL_FRONT, GL_LINE);
  glUseProgram(g_ShaderProgram);
  // glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

  // TODO: Look up what swapbuffers should be used
  SwapBuffers(DeviceContext);
#endif
}
static LRESULT CALLBACK win32_MainWindowCallback(HWND WindowHandle,
                                                 UINT Message, WPARAM WParam,
                                                 LPARAM LParam)
{
  LRESULT Result = 0;

  switch (Message)
  {
    case WM_CLOSE:
      {
        OutputDebugStringA("WM_CLOSE\n");
        g_Running = false;
        break;
      }

    case WM_DESTROY:
      {
        OutputDebugStringA("WM_DESTROY\n");
        g_Running = false;
        break;
      }

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
      {
        NeoAssert(!"NO INPUT HERE");
      }

    case WM_ACTIVATEAPP:
      {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
        break;
      }

    default:
      {
        Result = DefWindowProc(WindowHandle, Message, WParam, LParam);
      }
  }

  return Result;
}

static void win32_ProcessKeebMessage(engine_button_state *NewState, b32 IsDown)
{
  if (NewState->is_down != IsDown)
  {
    NewState->is_down = IsDown;
    ++NewState->half_transitions;
  }
}

static void win32_ProcessPendingWinMessages(engine_controller_input *Keyboard)
{
  MSG Message;
  while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
  {
    switch(Message.message)
    {
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP:
        {
          // virt keycode that tells us what the key was
          // Okay to truncate b/c vk_codes are not 64-bit![HHD016]
          u32 VkCode = (u32) Message.wParam;
          b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
          b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
          if (IsDown != WasDown)
          {
            switch (VkCode)
            {
              case 'W':
                {
                  win32_ProcessKeebMessage(&Keyboard->move_up, IsDown);
                } break;
              case 'A':
                {
                  win32_ProcessKeebMessage(&Keyboard->move_left, IsDown);
                } break;
              case 'S':
                {
                  win32_ProcessKeebMessage(&Keyboard->move_down, IsDown);
                } break;
              case 'D':
                {
                  win32_ProcessKeebMessage(&Keyboard->move_right, IsDown);
                } break;
              case 'Q':
                {
                  win32_ProcessKeebMessage(&Keyboard->left_shoulder, IsDown);
                } break;
              case 'E':
                {
                  win32_ProcessKeebMessage(&Keyboard->right_shoulder, IsDown);
                } break;
              case VK_UP:
                {
                  win32_ProcessKeebMessage(&Keyboard->action_up, IsDown);
                } break;
              case VK_DOWN:
                {
                  win32_ProcessKeebMessage(&Keyboard->action_down, IsDown);
                } break;
              case VK_LEFT:
                {
                  win32_ProcessKeebMessage(&Keyboard->action_left, IsDown);
                } break;
              case VK_RIGHT:
                {
                  win32_ProcessKeebMessage(&Keyboard->action_right, IsDown);
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
                  win32_ProcessKeebMessage(&Keyboard->start, IsDown);
                  // TODO: this really needs to be sent to our main menu
                  g_Running = false;
                } break;
              case VK_BACK:
                {
                  win32_ProcessKeebMessage(&Keyboard->back, IsDown);
                } break;
            }
          }

          b32 AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
          if ((VkCode == VK_F4) && AltKeyWasDown)
          {
            g_Running = false;
          }
        } break;

      default:
        {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } break;

    }

  }
}

INT WINAPI WinMain(HINSTANCE WinInstance, HINSTANCE PrevInstance,
                   PSTR CmdLine, INT ShowCommand)
{
  UINT DesiredSchedulerTiming = 1;
  b32 GranularSleep = (timeBeginPeriod(DesiredSchedulerTiming) == TIMERR_NOERROR);

  LARGE_INTEGER PerfCountFreq;
  QueryPerformanceFrequency(&PerfCountFreq);
  g_PerfCountFreq = PerfCountFreq.QuadPart;

  // Creates a window class that defines a window
  WNDCLASSA WinClass = {};
  WinClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // necessary flags to redraw the entire window.
  WinClass.lpfnWndProc = win32_MainWindowCallback;
  WinClass.hInstance = WinInstance;
  WinClass.lpszClassName = "MainWindowClass";

  win32_ResizeDIBSection(&g_BitmapBuffer, 960, 540);

  if (!RegisterClassA(&WinClass))
    return false;

  HWND Window = CreateWindowExA(0, WinClass.lpszClassName, "Blackjack Buddy",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0,
                                0, WinInstance, 0);
  if(!Window)
    return false;

  // Based on how the device context system functions, when specifying a
  // CS_OWNDC flag for the window, it is possible to get the DC for the window
  // and then never give it back because we now own it for the duration of the
  // program.
  HDC DeviceContext = GetDC(Window);

  // TODO: Variable refresh rate!
  int MonitorHz = 60;

  f32 UpdateHz = MonitorHz / 2.0f;
  f32 TargetSecondsPerFrame = 1.0f / UpdateHz;

#if NEO_INTERNAL
  LPVOID BaseAddress = (LPVOID) terabytes(2);
#else
  LPVOID BaseAddress = 0;
#endif

  app_memory AppMemory = {};
  AppMemory.perm_storage_space = megabytes(64);
  AppMemory.flex_storage_space = gigabytes(4);
  AppMemory.DEBUG_free_file = DEBUG_free_file;
  AppMemory.DEBUG_read_entire_file = DEBUG_read_entire_file;
  AppMemory.DEBUG_write_entire_file = DEBUG_write_entire_file;

  // TODO: Probably want to look into MEM_LARGE_PAGES?? HHD[024]
  u64 TotalSize = AppMemory.perm_storage_space + AppMemory.flex_storage_space;
  AppMemory.perm_mem_storage = VirtualAlloc(BaseAddress, TotalSize,
                                            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  AppMemory.flex_mem_storage = ((u8 *)AppMemory.perm_mem_storage + AppMemory.perm_storage_space);

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
  win32_InitOpengl(Window);

  engine_input Input[2] = {};
  engine_input *NewInput = &Input[0];
  engine_input *OldInput = &Input[1];

  // Global Loop
  g_Running = true;
  while (g_Running)
  {
    NewInput->time_step_over_frame = TargetSecondsPerFrame;

    engine_controller_input *NewKeeb = GetController(NewInput, 0);
    engine_controller_input *OldKeeb = GetController(OldInput, 0);
    *NewKeeb = {};

    for (ums ButtonIndex = 0;
    ButtonIndex < ArrayCount(NewKeeb->buttons);
    ButtonIndex++)
    {
      NewKeeb->buttons[ButtonIndex].is_down = OldKeeb->buttons[ButtonIndex].is_down;
    }

    win32_ProcessPendingWinMessages(NewKeeb);

    POINT MousePoint;
    GetCursorPos(&MousePoint);
    ScreenToClient(Window, &MousePoint);
    NewInput->mouseX = MousePoint.x;
    NewInput->mouseY = MousePoint.y;
    NewInput->mouseZ = 0;
    win32_ProcessKeebMessage(&NewInput->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
    win32_ProcessKeebMessage(&NewInput->mouse_buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
    win32_ProcessKeebMessage(&NewInput->mouse_buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));

    DWORD MaxControllerCount = KEEB_COUNT;
    if (MaxControllerCount > ArrayCount(NewInput->controllers)) {
      MaxControllerCount = ArrayCount(NewInput->controllers);
    }

    // First controller is the keyboard
    for (DWORD ControllerIndex = KEEB_COUNT;
    ControllerIndex < MaxControllerCount;
    ControllerIndex++)
    {
      engine_controller_input *OldControlState = GetController(OldInput, ControllerIndex);
      engine_controller_input *NewControlState = GetController(NewInput, ControllerIndex);
    }

    thread_context Thread = {};
    // This pulls from our platform-independent code from nebula.h
    engine_bitmap_buffer BitmapBuffer = {};
    BitmapBuffer.memory = g_BitmapBuffer.memory;
    BitmapBuffer.width = g_BitmapBuffer.width;
    BitmapBuffer.height = g_BitmapBuffer.height;
    BitmapBuffer.pitch = g_BitmapBuffer.pitch;
    BitmapBuffer.bytes_per_pixel = g_BitmapBuffer.bytes_per_pixel;
    UpdateAndRender(&Thread, &AppMemory, NewInput, &BitmapBuffer);

    win32_win_dimensions WindowDims = win32_GetWindowDims(Window);
    win32_UpdateWindownWithBuffer(DeviceContext, &BitmapBuffer,
                                  WindowDims.width, WindowDims.height);

    engine_input *Temp = NewInput;
    NewInput = OldInput;
    OldInput = Temp;
  }

  return 0;
}