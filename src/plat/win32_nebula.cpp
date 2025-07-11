#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "../core.unity.h"
#include "win32_nebula.h"

// Globals
global b32 g_Running = false;
global win32_bitmap_buffer g_BitmapBuffer;
global LPDIRECTSOUNDBUFFER g_SecondaryBuffer;
global s64 g_PerfCountFreq;
global loaded_bmp g_FontBitmap;

// typedef HGLRC WINAPI wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

inline static void *win32_AllocateMemory(ums Size)
{
  void *Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  return Result;

}
inline static void win32_DeallocateMemory(memory_arena *Arena)
{
  if (Arena->base)
  {
    VirtualFree(Arena->base, 0, MEM_RELEASE);
    Arena->base = 0;
  }
}

static void win32_InitOpengl(HWND WindowHandle, thread_context *Thread, app_memory *Memory, renderer *Renderer)
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
  glGenVertexArrays = (glgenvertexarrays *)wglGetProcAddress("glGenVertexArrays");
  glGenBuffers = (glgenbuffers *)wglGetProcAddress("glGenBuffers");
  glBindBuffer = (glbindbuffer *)wglGetProcAddress("glBindBuffer");
  glBufferData = (glbufferdata *)wglGetProcAddress("glBufferData");
  glBufferSubData = (glbuffersubdata *)wglGetProcAddress("glBufferSubData");
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
  glBindVertexArray = (glbindvertexarray *)wglGetProcAddress("glBindVertexArray");
  glGetUniformLocation = (glgetuniformlocation *)wglGetProcAddress("glGetUniformLocation");
  glUniform4f = (gluniform4f *)wglGetProcAddress("glUniform4f");
  glGenerateMipmap = (glgeneratemipmap *)wglGetProcAddress("glGenerateMipmap");
  glActiveTexture = (glactivetexture *)wglGetProcAddress("glActiveTexture");
  glUniform1i = (gluniform1i *)wglGetProcAddress("glUniform1i");
  glUniformMatrix4fv = (gluniformmatrix4fv *)wglGetProcAddress("glUniformMatrix4fv");

  // WGL extensions
  wglSwapIntervalEXT = (wglswapintervalext *)wglGetProcAddress("wglSwapIntervalEXT");

  // Turn off v-sync
  wglSwapIntervalEXT(0);

  // TODO: Look into how to remove fringe
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Create Shader Program
  g_ShaderProgram = CreateOpenGLShaderProgram(Thread, "..\\vert.glsl", "..\\frag.glsl");

  // Create a (V)ertex (B)uffer (O)bject and (V)ertex (A)rray (O)bject
  glGenVertexArrays(1, &Renderer->VAO);
  glGenBuffers(1, &Renderer->VBO);
  glGenBuffers(1, &Renderer->EBO);

  // Bind VAO, the bind and set VBOs, then config vertex attribs
  glBindVertexArray(Renderer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
  // NOTE: 4 is for Quads (vertices per quad) and 6 is the # of indices needed per quad.
  glBufferData(GL_ARRAY_BUFFER, Renderer->max_units*4*sizeof(vertex_data), 0, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, Renderer->max_units*6*sizeof(u32), 0, GL_DYNAMIC_DRAW);

  // Tell opengl how to interpret our vertex data by setting pointers to the attribs
  // pos attrib
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(f32), (void *)0);

  // color attrib
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(f32), (void *)(3*sizeof(f32)));
  // tex coord attrib
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9*sizeof(f32), (void *)(6*sizeof(f32)));
  // tex_id attrib
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9*sizeof(f32), (void *)(8*sizeof(f32)));

  glGenTextures(1, &Renderer->card_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Renderer->card_texture);

  // Setting Texture wrapping method
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // Setting Texture filtering method
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Renderer->card_atlas.width, Renderer->card_atlas.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Renderer->card_atlas.pixels);

  // Load font atlas
  u8 Pixels[512*512];
  s32 Width = 512;
  s32 Height = 512;

  debug_file_result TTFontFile = Memory->DEBUG_read_entire_file(Thread, "test/Code New Roman.otf");
  stbtt_BakeFontBitmap((u8 *)TTFontFile.contents, 0, 64.0f, Pixels, Width, Height, 32, 96, Renderer->chars);
  Memory->DEBUG_free_file(Thread, TTFontFile.contents);

  // NOTE: This expands out the single channel bitmap into one that
  // is four channels. Additionally, I am certain the bitmap is created
  // upside down but this is correct in the vertex_data for text.
  u8 *FontTexture = (u8 *)malloc(Width*Height*4);
  u8 *FTPtr = FontTexture;
  u8 *StbBitmap = Pixels;
  u32 Stride = 4*512;
  for (s32 Row = 0; Row < Height; ++Row)
  {
    u32 *Dest = (u32 *)FTPtr;
    for (s32 Col = 0; Col < Width; ++Col)
    {
      u8 MonoPixel = *StbBitmap++;
      *Dest++ = ((MonoPixel << 24) |
                 (MonoPixel << 16) |
                 (MonoPixel << 8)  |
                 (MonoPixel << 0));
    }
    FTPtr += Stride;
  }

  glGenTextures(1, &Renderer->font_texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, Renderer->font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, FontTexture);
  free(FontTexture);

  glUseProgram(g_ShaderProgram);
  glUniform1i(glGetUniformLocation(g_ShaderProgram, "CardTex"), 0);
  glUniform1i(glGetUniformLocation(g_ShaderProgram, "FontTex"), 1);

  // TODO: Set this up to use Right-handed coord system where:
  // -Y is right, up is +Z, and forward +X.
  mat4 Projection = Mat4Ortho(0.0f, (f32)Renderer->width, 0.0f, (f32)Renderer->height, -1.0f, 100.0f);
  mat4 Mvp = Projection*Mat4Iden()*Mat4Iden();
  Renderer->mvp = Mvp;

  // NOTE: Since I want to batch the quads and keep things simple, I am going to set a
  // default MVP since the transforms will be baked into the position data for each quad
  //
  // Does transformation in NDC.
  // mat4 TrueMvp = Unit->model*Renderer->mvp;
  //
  // Does transformation in screenspace.
  // NOTE: Since OGL is col-major, need to transpose here (GL_TRUE in the func below).
  glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "u_MVP"), 1, GL_TRUE, (f32 *)Mvp.e);

  // NOTE: I might unbind the buffers at some point.
  //
  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // glBindVertexArray(0);
  // glUseProgram(0);

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

inline LARGE_INTEGER win32_GetSecondsWallClock()
{
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return Result;
}

inline f32 win32_GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
  f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)g_PerfCountFreq);
  return Result;
}

// This is needed to force rePaint to get a set fps
static void win32_UpdateWindowWithBuffer(HDC DeviceContext,
                                          engine_bitmap_buffer *BMBuffer,
                                          int WindowWidth, int WindowHeight)
{
  // NOTE: Need to make a win32_bitmap_buffer when using this bliting
  // TODO: Probably need to make sure info points to something??
  win32_bitmap_buffer Buffer = {0};
  Buffer.memory = BMBuffer->memory;
  Buffer.pitch = BMBuffer->pitch;
  Buffer.width = BMBuffer->width;
  Buffer.height = BMBuffer->height;
  Buffer.bytes_per_pixel = BMBuffer->bytes_per_pixel;

  // Clear the unused client pixels to black
  PatBlt(DeviceContext, 0, Buffer.height, WindowWidth, WindowHeight, BLACKNESS);
  PatBlt(DeviceContext, Buffer.width, 0, WindowWidth, WindowHeight, BLACKNESS);

  // TODO:
  // Fix the aspect ratio with math (either through floating point or
  // integer)
  StretchDIBits(
    DeviceContext,
    // _x, _y, _width, _height, // Props for dest bitmap
    // _x, _y, _width, _height, // Props of src bitmap
    // To make a simple rect first, we  stretch the bits to the whole window
    // NOTE: HHD[024]: casey wanted to make the displayed pixels be one-to-one
    // so instead of taking in the win dimensions for this function,
    // we will be making it based on the Buffer's initial dims
    0, 0, Buffer.width, Buffer.height,
    0, 0, Buffer.width, Buffer.height,
    Buffer.memory, &Buffer.info,
    DIB_RGB_COLORS, // iUsage - Use literal rgb values to color in the pixels
    SRCCOPY);       // Raster Operation Code - Copy our bitmap to the dest
}

static void win32_UpdateWindow(HDC DeviceContext, renderer *Renderer,
                               int WindowWidth, int WindowHeight)
{
  // TODO: Make this into flags that are switched over for
  // which graphics API to render to.
  b32 IsHardware = true;
  if (IsHardware)
  {
    glViewport(0, 0, WindowWidth, WindowHeight);
    // TODO: Do aspect ratio
    //
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // gluPerspective(45.0, 16.0/9.0*float(win_width)/float(win_height), 0.1, 100.0);

    v3 ClearColor = Renderer->clear_color;
    glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // DRAW
    render_unit *Unit = Renderer->head;
    render_unit *PrevUnit = Renderer->head;
    for (u32 UnitCount = 0; UnitCount < Renderer->unit_count;)
    {
      do
      {
        s32 DataSize = (s32)Unit->vertex_count*sizeof(vertex_data);
        s32 DataOffset = UnitCount*DataSize;
        s32 IndexSize = (s32)Unit->index_count*sizeof(u32);
        s32 IndexOffset = UnitCount*IndexSize;
        glBufferSubData(GL_ARRAY_BUFFER, DataOffset, DataSize, Unit->vertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, IndexOffset, IndexSize, Unit->indices);
        ++UnitCount;
        if (Unit != PrevUnit)
        {
          PrevUnit = PrevUnit->next;
        }
        Unit = Unit->next;
      }
      while (Unit != 0 && Unit->vertices[0].tex_id == PrevUnit->vertices[0].tex_id);
      glDrawElements(GL_TRIANGLES, UnitCount*6, GL_UNSIGNED_INT, 0);
    }


    // TODO: Expand how we catch OGL errors
    NeoAssert(glGetError() == GL_NO_ERROR);

    // TODO: Look up what swapbuffers should be used
    SwapBuffers(DeviceContext);
  }
  else
  {
    // TODO: Replace this with other render apis
    engine_bitmap_buffer *BMBuffer = 0;
    win32_UpdateWindowWithBuffer(DeviceContext, BMBuffer,
                                 WindowWidth, WindowHeight);
  }

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
        break;
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

static void win32_ProcessPendingWinMessages(app_memory *Memory, engine_controller_input *Keyboard)
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
                  // if (Memory->is_init)
                  // {
                  //   app_state *GameState = (app_state *)Memory->perm_memory;
                  //   char OutStr[256];
                  //   card *Cards = GameState->base_deck.cards;
                  //   for (s32 Index = 0; Index < 4; ++Index)
                  //   {
                  //     Shuffle(Cards, ArrayCount(GameState->base_deck.cards));
                  //   }
                  //   // NewHandDeal(&GameState->base_deck);
                  //   for (s32 Index = 0; Index < 13; ++Index)
                  //   {
                  //     _snprintf_s(OutStr, sizeof(OutStr), "RANK: %d SUIT: %s\n",
                  //                 Cards[Index].value, Cards[Index].suit);
                  //     OutputDebugStringA(OutStr);
                  //   }
                  // }
                } break;
              case VK_ESCAPE:
                {
                  g_Running = false;
                } break;
              case VK_RETURN:
                {
                  win32_ProcessKeebMessage(&Keyboard->start, IsDown);
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

  // NOTE: Only expecting to have one renderer. This could change in the future.
  renderer Renderer = {0};
  Renderer.width = 1600;
  Renderer.height = 900;
  win32_ResizeDIBSection(&g_BitmapBuffer, Renderer.width, Renderer.height);

  if (!RegisterClassA(&WinClass))
    return false;

  RECT WindowRect = { 0, 0, (s32)Renderer.width, (s32)Renderer.height };
  DWORD DwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX | WS_VISIBLE;
  AdjustWindowRectEx(&WindowRect, DwStyle, FALSE, 0);

  HWND Window = CreateWindowExA(0, WinClass.lpszClassName, "Blackjack Buddy",
                                DwStyle, CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                WindowRect.right - WindowRect.left,
                                WindowRect.bottom - WindowRect.top, 0,
                                0, WinInstance, 0);
  if(!Window)
    return false;

  thread_context Thread = {};
  // Based on how the device context system functions, when specifying a
  // CS_OWNDC flag for the window, it is possible to get the DC for the window
  // and then never give it back because we now own it for the duration of the
  // program.
  HDC DeviceContext = GetDC(Window);

  // TODO: Variable refresh rate!
  s32 MonitorHz = 120;

  f32 UpdateHz = (f32)MonitorHz;
  f32 TargetSecondsPerFrame = 1.0f / UpdateHz;

#if NEO_INTERNAL
  LPVOID BaseAddress = (LPVOID) TB(1);
#else
  LPVOID BaseAddress = 0;
#endif

  s32 Err = 0;
  app_memory AppMemory = {0};
  AppMemory.perm_storage_size = MB(64);
  AppMemory.flex_storage_size = GB(1);
  AppMemory.DEBUG_free_file = DEBUG_free_file;
  AppMemory.DEBUG_read_entire_file = DEBUG_read_entire_file;
  AppMemory.DEBUG_write_entire_file = DEBUG_write_entire_file;

  // TODO: Probably want to look into MEM_LARGE_PAGES?? HHD[024]
  u64 TotalSize = AppMemory.perm_storage_size + AppMemory.flex_storage_size;
  AppMemory.perm_memory = VirtualAlloc(BaseAddress, TotalSize,
                                            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (AppMemory.perm_memory == 0) Err = GetLastError();
  AppMemory.flex_memory = ((u8 *)AppMemory.perm_memory + AppMemory.perm_storage_size);

#if 0
  loaded_jpg crate_tex = {};
  crate_tex = DEBUG_load_jpg(&global_arena, &g_thread_context, DEBUG_read_entire_file, "test/cardback.jpeg", DEBUG_free_file);
  if (crate_tex.pixels)
  {
    // Opengl
    win32_init_opengl(window, crate_tex);
  }
#endif

  // TODO: Figure out how to init each graphics API
  // in order to allow us to easily switch between
  // the different APIs.
  InitRenderer(&Thread, &AppMemory, &Renderer);

  // Init OpenGL
  win32_InitOpengl(Window, &Thread, &AppMemory, &Renderer);

  engine_input Input[2] = {0};
  engine_input *NewInput = &Input[0];
  engine_input *OldInput = &Input[1];

  // Global Loop
  g_Running = true;
  LARGE_INTEGER StartCounter = win32_GetSecondsWallClock();

  while (g_Running)
  {
    NewInput->time_step_over_frame = TargetSecondsPerFrame;

    engine_controller_input *NewKeeb = GetController(NewInput);
    engine_controller_input *OldKeeb = GetController(OldInput);
    *NewKeeb = {0};

    for (ums ButtonIndex = 0;
    ButtonIndex < ArrayCount(NewKeeb->buttons);
    ButtonIndex++)
    {
      NewKeeb->buttons[ButtonIndex].is_down = OldKeeb->buttons[ButtonIndex].is_down;
    }

    win32_ProcessPendingWinMessages(&AppMemory, NewKeeb);

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
      // TODO: Figure out what we are doing with the input here... and maybe in general...
      engine_controller_input *OldControlState = GetController(OldInput, ControllerIndex);
      engine_controller_input *NewControlState = GetController(NewInput, ControllerIndex);
    }

    UpdateAndRender(&Thread, &AppMemory, NewInput, &Renderer);
    {
      app_state *GameState = (app_state *)AppMemory.perm_memory;
      if (GameState->scene == sc_Quit)
      {
        g_Running = false;
      }
    }

    // NOTE: TIMING OUR RUNNING LOOP
    LARGE_INTEGER CurrentCounter = win32_GetSecondsWallClock();
    f32 CurrentSecondsElapsed = win32_GetSecondsElapsed(StartCounter, CurrentCounter);

    // Stall out if frame is ready sooner than target time
    f32 SecondsElapsedForFrame = CurrentSecondsElapsed;
    if (SecondsElapsedForFrame < TargetSecondsPerFrame)
    {
      if (GranularSleep)
      {
        // NOTE:
        // For some reason, the sleep is not accurate enough to wake in time. [HHD018]
        // Thus, I am sleeping 1ms less to make sure we lock at desired frame time.
        // DWORD SleepTime = (DWORD) ((1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame)) - 1.0f);
        DWORD SleepTime = (DWORD) ((1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame)));
        if (SleepTime > 0)
        {
          Sleep(SleepTime);
        }
      }

      // f32 TestSecondsElapsedForFrame = win32_GetSecondsElapsed(CurrentCounter, win32_GetSecondsWallClock());
      // if (TestSecondsElapsedForFrame < TargetSecondsPerFrame)
      // {
      //   // TODO: LOGGING HERE!
      //   OutputDebugStringA("MissedSleep!\n");
      // }

      while (SecondsElapsedForFrame < TargetSecondsPerFrame)
      {
        SecondsElapsedForFrame = win32_GetSecondsElapsed(StartCounter, win32_GetSecondsWallClock());
      }
    }
    else
    {
      // TODO: Logging here
      // Took too long to process current frame!
    }

    LARGE_INTEGER EndCounter = win32_GetSecondsWallClock();
    f32 TotalSecondsForFrame = win32_GetSecondsElapsed(StartCounter, EndCounter);
    StartCounter = EndCounter;

    b32 DebugOn = false;
    if (DebugOn)
    {
      // FrameTime
      f32 MSPerFrame = 1000.0f * TotalSecondsForFrame;
      MSPerFrame = truncf(MSPerFrame * 100.0f) / 100.0f;
      char TextContainer[256];
      _snprintf_s(TextContainer, sizeof(TextContainer), "FT: %.3f", MSPerFrame);
      mat4 Transform = Mat4Translate((f32)Renderer.width - 150.0f, (f32)Renderer.height - 20.0f, 0.0f)*Mat4Scale(0.35f, 0.35f, 1.0f);
      PushText(&Renderer, StrFromCStr(TextContainer), Transform);

      // FPS
      f32 FPS = 1000.0f / MSPerFrame;
      _snprintf_s(TextContainer, sizeof(TextContainer), "FPS: %.0f", FPS);
      Transform = Mat4Translate((f32)Renderer.width - 150.0f, (f32)Renderer.height - 50.0f, 0.0f)*Mat4Scale(0.35f, 0.35f, 1.0f);
      PushText(&Renderer, StrFromCStr(TextContainer), Transform);
    }

    win32_win_dimensions WindowDims = win32_GetWindowDims(Window);
    win32_UpdateWindow(DeviceContext, &Renderer, WindowDims.width, WindowDims.height);

    engine_input *Temp = NewInput;
    NewInput = OldInput;
    OldInput = Temp;
  }

  return 0;
}
