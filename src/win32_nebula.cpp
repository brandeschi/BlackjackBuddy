#include <math.h>
#include <stdint.h>

// TODO: THIS FILE WILL NEED DIAGS ONCE A DIAG SYSTEM IS MADE

#define global static
#define pi32 3.14159265359f
#define KEEB_COUNT 1

#include "nebula.h"
#include "nebula.cpp"

#include <Windows.h>
#include <stdio.h>
#include <Xinput.h>
#include <dsound.h>

#include "win32_nebula.h"

global b32 g_running = false;
global win32_bitmap_buffer g_bm_buffer;
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
global i64 g_perf_count_freq;

static debug_file_result DEBUG_read_entire_file(thread_context *thread, char *file_name)
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

static b32 DEBUG_write_entire_file(thread_context *thread, char *file_name, void *file, u32 file_size)
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

static void DEBUG_free_file(thread_context *thread, void *file)
{
    if (file)
    {
        VirtualFree(&file, 0, MEM_RELEASE);
    }
}

// Macros to get the two function declarations that we need to make xInput work.
#define XINPUT_GET_STATE(fn_name) DWORD WINAPI fn_name(DWORD dwUserIndex, XINPUT_STATE *pState) // Macro to define a function that matches the signature of XInputGetState
#define XINPUT_SET_STATE(fn_name) DWORD WINAPI fn_name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) // Macro to define a function that matche the signature of XInputSetState

// Define the macros above as types to make them into pointers.
typedef XINPUT_GET_STATE(x_input_get_state);
typedef XINPUT_SET_STATE(x_input_set_state);

// Stub functions that are pointed to when xInput is not loaded so the engine
// doesn't crash.
XINPUT_GET_STATE(XInputGetStateStubFn) { return (ERROR_DEVICE_NOT_CONNECTED); }

XINPUT_SET_STATE(XInputSetStateStubFn) { return (ERROR_DEVICE_NOT_CONNECTED); }

// These are used to make pointers to the stubs when xInput is not loaded.
global x_input_get_state *xinput_get_state = XInputGetStateStubFn;
global x_input_set_state *xinput_set_state = XInputSetStateStubFn;

// This allows for same function calls as if we were always using xInput.
#define XInputGetState xinput_get_state
#define XInputSetState xinput_set_state

static void win32_load_xinput() {
  HMODULE xinput_lib = LoadLibraryA("xinput9_1_0.dll");
  if (!xinput_lib) {
    xinput_lib = LoadLibraryA("xinput1_3.dll");
    // TODO: Add some kind of logging to let us know we didn't load xInputLib
  }
  if (xinput_lib) {
    XInputGetState =
        (x_input_get_state *)GetProcAddress(xinput_lib, "XInputGetState");
    XInputSetState =
        (x_input_set_state *)GetProcAddress(xinput_lib, "XInputSetState");
  }
}

// TODO: Should I replace this once I want to actually add sound?
#define DIRECT_SOUND_CREATE(fn_name) HRESULT WINAPI fn_name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter) // Macro to define a function that matches the signature of DirectSound Create func
typedef DIRECT_SOUND_CREATE(dsound_create);

static void win32_init_dsound(HWND win_handle, i32 buffer_size,
                              i32 samples_per_sec) {
  // NOTE: Load Lib
  HMODULE dsound_lib = LoadLibraryA("dsound.dll");
  if (dsound_lib) {
    // NOTE: Get DSound object (direct sound is OOP)
    dsound_create *direct_sound_create =
        (dsound_create *)GetProcAddress(dsound_lib, "DirectSoundCreate");

    LPDIRECTSOUND direct_sound;
    if (direct_sound_create &&
        SUCCEEDED(direct_sound_create(0, &direct_sound, 0))) {
      WAVEFORMATEX wave_format = {};
      wave_format.wFormatTag = WAVE_FORMAT_PCM;
      wave_format.nChannels = 2;
      wave_format.nSamplesPerSec = samples_per_sec;
      wave_format.wBitsPerSample = 16; // 16-bit audio
      wave_format.nBlockAlign =
          (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
      wave_format.nAvgBytesPerSec =
          wave_format.nSamplesPerSec * wave_format.nBlockAlign;
      wave_format.cbSize = 0;

      // TODO: assert this later
      if (SUCCEEDED(
              direct_sound->SetCooperativeLevel(win_handle, DSSCL_PRIORITY))) {
        // NOTE: Create a 'primary buffer'
        DSBUFFERDESC buffer_desc = {};
        buffer_desc.dwSize = sizeof(buffer_desc);
        buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        // Could this be an issue that dwBufferBytes expects and ulong (32bits)
        // while we pass it a int32?
        LPDIRECTSOUNDBUFFER primary_buffer;
        if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                      &primary_buffer, 0))) {
          if (SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
            // NOTE: Format set!
            OutputDebugStringA("primary buffer set!\n");
          }
        }
      }
      // NOTE: Create a 'secondary buffer'
      DSBUFFERDESC buffer_desc = {};
      buffer_desc.dwSize = sizeof(buffer_desc);
      buffer_desc.dwFlags = 0;
      buffer_desc.dwBufferBytes = buffer_size;
      buffer_desc.lpwfxFormat = &wave_format;
      if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                    &g_secondary_buffer, 0))) {
        OutputDebugStringA("second buffer set!\n");
      }
    }

    // NOTE: Play!
  } else {
    // TODO: DIAG
  }
}

static win32_win_dimensions win32_get_win_dimensions(HWND win_handle) {
  win32_win_dimensions window_dimensions = {};
  RECT client_rect;
  GetClientRect(win_handle, &client_rect);

  window_dimensions.width = client_rect.right - client_rect.left;
  window_dimensions.height = client_rect.bottom - client_rect.top;

  return window_dimensions;
}

static void win32_resize_DIB_section(win32_bitmap_buffer *buffer, int _width,
                                     int _height) {
  // Need to free the memory but only when it will be reallocated right away
  // since a paint will happen every frame
  if (buffer->memory) {
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
// This is needed to force rePaint to get a set fps
static void win32_update_win_with_buffer(HDC device_context,
                                         win32_bitmap_buffer *buffer,
                                         int win_width, int win_height)
{
    // Clear the unused client pixels to black
    PatBlt(device_context, 0, buffer->height, win_width, win_height, BLACKNESS);
    PatBlt(device_context, buffer->width, 0, win_width, win_height, BLACKNESS);

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
        // we will be making it based on the buffer's initial dims
        0, 0, buffer->width, buffer->height, 0, 0, buffer->width, buffer->height,
        buffer->memory, &buffer->info,
        DIB_RGB_COLORS, // iUsage - Use literal rgb values to color in the pixels
        SRCCOPY);       // Raster Operation Code - Copy our bitmap to the dest
}
static LRESULT CALLBACK win32_main_window_callback(HWND win_handle,
                                                   UINT message, WPARAM WParam,
                                                   LPARAM LParam)
{
  LRESULT result = 0;

  switch (message)
  {
      case WM_CLOSE: {
        OutputDebugStringA("WM_CLOSE");
        g_running = false;
        break;
      }

      case WM_DESTROY: {
        OutputDebugStringA("WM_DESTROY");
        g_running = false;
        break;
      }

      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        assert(!"NO INPUT HERE");
      }

      case WM_PAINT: {
        PAINTSTRUCT s_paint;
        HDC device_context = BeginPaint(win_handle, &s_paint);
        win32_win_dimensions win_size = win32_get_win_dimensions(win_handle);
        win32_update_win_with_buffer(device_context, &g_bm_buffer, win_size.width,
                                     win_size.height);
        EndPaint(win_handle, &s_paint);
        break;
      }

      case WM_ACTIVATEAPP: {
        OutputDebugStringA("WM_ACTIVATEAPP");
        break;
      }

      default: {
        // OutputDebugStringA("default");
        result = DefWindowProc(win_handle, message, WParam, LParam);
      }
  }

  return result;
}

static void win32_clear_sound_buffer(win32_sound_settings *sound_settings) {
  VOID *region1;
  DWORD region1_size;
  VOID *region2;
  DWORD region2_size;

  if (SUCCEEDED(g_secondary_buffer->Lock(0, sound_settings->secondary_buffer_size,
                                       &region1, &region1_size, &region2,
                                       &region2_size, 0))) {
    // Clear region 1
    u8 *dest_samples = (u8 *)region1;
    for (DWORD byte_index = 0; byte_index < region1_size; byte_index++) {
      *dest_samples++ = 0;
    }

    // Clear region 2
    dest_samples = (u8 *)region2;
    for (DWORD byte_index = 0; byte_index < region2_size; byte_index++) {
      *dest_samples++ = 0;
    }

    g_secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
  }
}

static void win32_fill_sound_buffer(win32_sound_settings *sound_settings,
                                    DWORD bytes_to_lock, DWORD bytes_to_write,
                                    engine_sound_buffer *source_buffer) {
  VOID *region1;
  DWORD region1_size;
  VOID *region2;
  DWORD region2_size;

  if (SUCCEEDED(g_secondary_buffer->Lock(bytes_to_lock, bytes_to_write, &region1,
                                       &region1_size, &region2, &region2_size,
                                       0))) {
    // TODO: assert region sizes
    DWORD region1_sample_count =
        region1_size / sound_settings->bytes_per_sample;
    i16 *dest_samples = (i16 *)region1;
    i16 *source_samples = source_buffer->samples;

    for (DWORD sample_index = 0; sample_index < region1_sample_count;
         sample_index++) {
      *dest_samples++ = *source_samples++;
      *dest_samples++ = *source_samples++;
      sound_settings->running_sample_index++;
    }

    DWORD region2_sample_count =
        region2_size / sound_settings->bytes_per_sample;
    dest_samples = (i16 *)region2;
    for (DWORD sample_index = 0; sample_index < region2_sample_count;
         sample_index++) {
      *dest_samples++ = *source_samples++;
      *dest_samples++ = *source_samples++;
      sound_settings->running_sample_index++;
    }

    g_secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
  }
}

static float win32_process_stick_value(SHORT thumb_stick_value, SHORT deadzone_threshold)
{
    float result = 0;
    if (thumb_stick_value < -deadzone_threshold) {
      result = (float) (thumb_stick_value - deadzone_threshold) / 32768.0f;
    } else if (thumb_stick_value > deadzone_threshold) {
      result = (float) (thumb_stick_value - deadzone_threshold) / 32767.0f;
    }

    return result;
}

static void win32_xinput_digital_button_handler(DWORD xinput_button_state,
                                                engine_button_state *old_state,
                                                engine_button_state *new_state,
                                                DWORD button_bit) {
  new_state->is_down = ((xinput_button_state & button_bit) == button_bit);
  new_state->half_transitions =
      (old_state->is_down != new_state->is_down) ? 1 : 0;
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
                        case 'W': {
                            win32_process_keeb_message(&keyboard->move_up, is_down);
                          break;
                        }
                        case 'A': {
                            win32_process_keeb_message(&keyboard->move_left, is_down);
                          break;
                        }
                        case 'S': {
                            win32_process_keeb_message(&keyboard->move_down, is_down);
                          break;
                        }
                        case 'D': {
                            win32_process_keeb_message(&keyboard->move_right, is_down);
                          break;
                        }
                        case 'Q': {
                            win32_process_keeb_message(&keyboard->left_shoulder, is_down);
                          break;
                        }
                        case 'E': {
                            win32_process_keeb_message(&keyboard->right_shoulder, is_down);
                          break;
                        }
                        case VK_UP: {
                            win32_process_keeb_message(&keyboard->action_up, is_down);
                          break;
                        }
                        case VK_DOWN: {
                            win32_process_keeb_message(&keyboard->action_down, is_down);
                          break;
                        }
                        case VK_LEFT: {
                            win32_process_keeb_message(&keyboard->action_left, is_down);
                          break;
                        }
                        case VK_RIGHT: {
                            win32_process_keeb_message(&keyboard->action_right, is_down);
                          break;
                        }
                        case VK_SPACE: {
                            char put_string[256];
                            // for (i32 i = 0; i < arr_count(base_deck.cards); i++)
                            // {
                            //     _snprintf_s(put_string, sizeof(put_string), "RANK: %d SUIT: %s\n",
                            //                 base_deck.cards[i].Value, base_deck.cards[i].Suit);
                            //     OutputDebugStringA(put_string);
                            // }
                            // OutputDebugStringA("==============================\n");
                            // shuffle(base_deck.cards, arr_count(base_deck.cards));
                            // for (i32 i = 0; i < arr_count(base_deck.cards); i++)
                            // {
                            //     _snprintf_s(put_string, sizeof(put_string), "RANK: %d SUIT: %s\n",
                            //                 base_deck.cards[i].Value, base_deck.cards[i].Suit);
                            //     OutputDebugStringA(put_string);
                            // }
                            // OutputDebugStringA("SPACE\n");
                            deal(&base_deck, 2);
                            _snprintf_s(put_string, sizeof(put_string), "RANK: %d SUIT: %s\n",
                                        base_deck.cards[4].Value, base_deck.cards[4].Suit);
                            OutputDebugStringA(put_string);
                            break;
                        }
                        case VK_ESCAPE: {
                          win32_process_keeb_message(&keyboard->start, is_down);
                          // TODO: this really needs to be sent to our main menu
                          g_running = false;
                          break;
                        }
                        case VK_BACK: {
                          win32_process_keeb_message(&keyboard->back, is_down);
                          break;
                        }
                    }
                }

                b32 alt_key_was_down = ((message.lParam & (1 << 29)) != 0);
                if ((vk_code == VK_F4) && alt_key_was_down) {
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

#if 0
static void win32_draw_debug_verticals(win32_bitmap_buffer *bm_buffer, int current_x,
                                       int top, int bot, int color)
{
    if (top <= 0)
    {
        top = 0;
    }
    if (bot > bm_buffer->height)
    {
        bot = bm_buffer->height;
    }

    if ((current_x >= 0) && (current_x < bm_buffer->width))
    {
        u8 *pixel = ((u8 *) bm_buffer->memory +
                     (current_x * bm_buffer->bytes_per_pixel) +
                     (top * bm_buffer->pitch));
        for (int y = top; y < bot; y++)
        {
            *(u32 *)pixel = color;
            pixel += bm_buffer->pitch;
        }
    }
}

static void win32_render_debug_display_sync(win32_bitmap_buffer *bm_buffer, int debug_cursor_count,
                                            DWORD *debug_cursors, win32_sound_settings *sound_settings,
                                            f32 target_seconds_per_frame)
{
    int pad_x = 32;
    int pad_y = 32;

    int top = pad_y;
    int bot = bm_buffer->height - pad_y;

    f32 coeff = (f32) (bm_buffer->width - pad_x) / (f32) sound_settings->secondary_buffer_size;

    for (int debug_cursor_index = 0;
         debug_cursor_index < debug_cursor_count;
         debug_cursor_index++)
    {
        int current_x = pad_x + (int) (coeff * (f32) debug_cursors[debug_cursor_index]);
        win32_draw_debug_verticals(bm_buffer, current_x, top, bot, 0xFFFFFF);
    }
}
#endif

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

INT WINAPI WinMain(HINSTANCE win_instance, HINSTANCE prev_instance,
                   PSTR cmd_line, INT show_command)
{
    UINT desired_scheduler_timing = 1;
    b32 granular_sleep = (timeBeginPeriod(desired_scheduler_timing) == TIMERR_NOERROR);

    LARGE_INTEGER perf_count_freq;
    QueryPerformanceFrequency(&perf_count_freq);
    g_perf_count_freq = perf_count_freq.QuadPart;

    // Creates a window class that defines what a window is
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

    // Based on how the device context system functions, when specifying a
    // CS_OWNDC flag for the window, it is possible to get the DC for the window
    // and then never give it back because we now own it for the duration of the
    // program.
    HDC device_context = GetDC(window);

    // NOTE: Due to having a higher than 60hz display, I will figure out the GetDeviceCaps call later!
    int monitor_hz = 60;

    f32 update_hz = monitor_hz / 2.0f;
    f32 target_seconds_per_frame = 1.0f / update_hz;

    // Load input
    win32_load_xinput();

    // Set up our sound config
    win32_sound_settings sound_settings = {};
    sound_settings.samples_per_sec = 48000;
    sound_settings.bytes_per_sample = sizeof(i16) * 2;
    sound_settings.secondary_buffer_size =
      sound_settings.samples_per_sec * sound_settings.bytes_per_sample;
    // NOTE: Seems that the latency is at least 3 times our samples/update cycle.
    // TODO: Need to calculate the variance on the sound to find what the proper value of cushion we need.
    sound_settings.cushion_bytes = (DWORD)(((f32)(sound_settings.samples_per_sec * sound_settings.bytes_per_sample) / update_hz) / 3.0f);

    // Start up directsound
    win32_init_dsound(window, sound_settings.secondary_buffer_size,
                    sound_settings.samples_per_sec);
    win32_clear_sound_buffer(&sound_settings);
    g_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

    // TODO:
    // Pool this with bitmap also, once I have an idea about how I want the core of this engine to be,
    // I might want to change how I do these virtualallocs (especially if casey never gets to it)
    i16 *samples =
      (i16 *)VirtualAlloc(0, sound_settings.secondary_buffer_size,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // TODO:
    // For now, we are just setting up the memory space for my machine.
    // Later this will need to handled to in some other way by looking at what we
    // are running on.
#if NEO_INTERNAL
    LPVOID base_address = (LPVOID) terabytes(2);
#else
    LPVOID base_address = 0;
#endif

    app_memory app_memory = {};
    app_memory.perm_storage_space = megabytes(64);
    app_memory.flex_storage_space = gigabytes(4);

    // TODO: Probably want to look into MEM_LARGE_PAGES?? HHD[024]
    u64 total_size = app_memory.perm_storage_space + app_memory.flex_storage_space;
    app_memory.perm_mem_storage = VirtualAlloc(
      base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    app_memory.flex_mem_storage =
      ((u8 *) app_memory.perm_mem_storage + app_memory.perm_storage_space);

    // TODO: Add check here to make sure we got our memory(samples, bitmap, app_mem)
    engine_input input[2] = {};
    engine_input *new_input = &input[0];
    engine_input *old_input = &input[1];

    g_running = true;
    LARGE_INTEGER start_counter = win32_get_seconds_wallclock();

    int debug_cursor_index = 0;
    DWORD debug_cursors[30] = {};

    DWORD audio_latency_bytes = 0;
    f32 audio_latency_seconds = 0.0f;
    b32 valid_sound = false;

    // Global Loop
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

        // NOTE: since xInput polls the inputs, need to make sure if more polling is
        // needed/control over the polling is needed for the engine.
        POINT mouse_point;
        GetCursorPos(&mouse_point);
        ScreenToClient(window, &mouse_point);
        new_input->mouseX = mouse_point.x;
        new_input->mouseY = mouse_point.y;
        new_input->mouseZ = 0;
        win32_process_keeb_message(&new_input->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
        win32_process_keeb_message(&new_input->mouse_buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
        win32_process_keeb_message(&new_input->mouse_buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));

        DWORD max_controller_count = KEEB_COUNT + XUSER_MAX_COUNT;
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

            XINPUT_STATE controller_state;
            ZeroMemory(&controller_state, sizeof(XINPUT_STATE));

            // Simply get the state of the controller from XInput.
            // TODO: See if XInputGetState still has the bug discussed in HHD007.
            if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
            {
                new_control_state->is_analog = old_control_state->is_analog;

                // Controller is connected
                XINPUT_GAMEPAD *controller = &controller_state.Gamepad;

                // TODO: Potentially change to 'keyboard' control-like code path by adding is_analog = false for d-pad
                // DPAD
                if (controller->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                {
                    new_control_state->stick_avg_y = -1.0f;
                    new_control_state->is_analog = false;
                }
                if (controller->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                {
                    new_control_state->stick_avg_y = 1.0f;
                    new_control_state->is_analog = false;
                }
                if (controller->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                {
                    new_control_state->stick_avg_x = -1.0f;
                    new_control_state->is_analog = false;
                }
                if (controller->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                {
                    new_control_state->stick_avg_x = 1.0f;
                    new_control_state->is_analog = false;
                }

                new_control_state->is_analog = true;
                // LSTICK
                new_control_state->stick_avg_x =
                    win32_process_stick_value(controller->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                new_control_state->stick_avg_y =
                    win32_process_stick_value(controller->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                float threshold = 0.5f;
                win32_xinput_digital_button_handler(
                    (new_control_state->stick_avg_y < -threshold ? 1 : 0),
                    &old_control_state->move_down,
                    &new_control_state->move_down, 1);
                win32_xinput_digital_button_handler(
                    (new_control_state->stick_avg_x < -threshold ? 1 : 0),
                    &old_control_state->move_left,
                    &new_control_state->move_left, 1);
                win32_xinput_digital_button_handler(
                    (new_control_state->stick_avg_y > threshold ? 1 : 0),
                    &old_control_state->move_up,
                    &new_control_state->move_up, 1);
                win32_xinput_digital_button_handler(
                    (new_control_state->stick_avg_x > threshold ? 1 : 0),
                    &old_control_state->move_right,
                    &new_control_state->move_right, 1);

                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->action_down,
                    &new_control_state->action_down, XINPUT_GAMEPAD_A);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->action_right,
                    &new_control_state->action_right, XINPUT_GAMEPAD_B);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->action_left,
                    &new_control_state->action_left, XINPUT_GAMEPAD_X);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->action_up,
                    &new_control_state->action_up, XINPUT_GAMEPAD_Y);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->left_shoulder,
                    &new_control_state->left_shoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->right_shoulder,
                    &new_control_state->right_shoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->start,
                    &new_control_state->start, XINPUT_GAMEPAD_START);
                win32_xinput_digital_button_handler(
                    controller->wButtons, &old_control_state->back,
                    &new_control_state->back, XINPUT_GAMEPAD_BACK);

                /* b32 left_thumb_button = (controller->wButtons &
                 * XINPUT_GAMEPAD_LEFT_THUMB); */
                /* b32 right_thumb_button = (controller->wButtons &
                 * XINPUT_GAMEPAD_RIGHT_THUMB); */
            }
            else
            {
                // TODO: Add basic keyboard controls next.
                // Controller is not connected
            }
        }

        thread_context thread = {};
        // This pulls from our platform-independent code from nebula.h
        engine_bitmap_buffer buffer = {};
        buffer.memory = g_bm_buffer.memory;
        buffer.width = g_bm_buffer.width;
        buffer.height = g_bm_buffer.height;
        buffer.pitch = g_bm_buffer.pitch;
        buffer.bytes_per_pixel = g_bm_buffer.bytes_per_pixel;
        update_and_render(&thread, &app_memory, new_input, &buffer);

        // NOTE: Go to HHD020 to see comment about how audio sync will work.

        // SOUND
        DWORD play_cursor = 0;
        DWORD write_cursor = 0;
        if (SUCCEEDED(g_secondary_buffer->GetCurrentPosition(&play_cursor,
                                                           &write_cursor)))
        {
            if (!valid_sound)
            {
                sound_settings.running_sample_index = write_cursor / sound_settings.bytes_per_sample;
                valid_sound = true;
            }

            DWORD bytes_to_lock = ((sound_settings.running_sample_index *
                              sound_settings.bytes_per_sample) %
                             sound_settings.secondary_buffer_size);


            DWORD expected_sound_bytes_per_frame = (DWORD)((f32)(sound_settings.samples_per_sec * sound_settings.bytes_per_sample) / update_hz);
            DWORD expected_frame_boundary_byte = play_cursor + expected_sound_bytes_per_frame;

            DWORD safe_write_cursor = write_cursor;
            if (safe_write_cursor < play_cursor)
            {
                safe_write_cursor += sound_settings.secondary_buffer_size;
            }
            safe_write_cursor += sound_settings.cushion_bytes;
            assert(safe_write_cursor >= play_cursor);

            b32 latent_audio = (safe_write_cursor >= expected_frame_boundary_byte);
            DWORD target_cursor = 0;
            if (latent_audio)
            {
                target_cursor = write_cursor + expected_sound_bytes_per_frame + sound_settings.cushion_bytes;
            }
            else
            {
                target_cursor = expected_frame_boundary_byte + expected_sound_bytes_per_frame;
            }
            target_cursor = target_cursor % sound_settings.secondary_buffer_size;


            DWORD bytes_to_write = 0;
            if (bytes_to_lock > target_cursor)
            {
                bytes_to_write = sound_settings.secondary_buffer_size - bytes_to_lock;
                bytes_to_write += target_cursor;
            }
            else
            {
                bytes_to_write = target_cursor - bytes_to_lock;
            }

            engine_sound_buffer sound_buffer = {};
            sound_buffer.samples_per_second = sound_settings.samples_per_sec;
            sound_buffer.sample_count =
                bytes_to_write / sound_settings.bytes_per_sample;
            sound_buffer.samples = samples;
            app_get_sound_samples(&thread, &app_memory, &sound_buffer);

            win32_fill_sound_buffer(&sound_settings, bytes_to_lock, bytes_to_write,
                                  &sound_buffer);
        }
        else
        {
            valid_sound = false;
        }

        // NOTE: TIMING OUR RUNNING LOOP
        LARGE_INTEGER current_counter = win32_get_seconds_wallclock();
        f32 current_seconds_elapsed = win32_get_seconds_elapsed(start_counter, current_counter);

        // Stall out if frame is ready sooner than target time
        f32 secs_elapsed_for_frame = current_seconds_elapsed;
        if (secs_elapsed_for_frame < target_seconds_per_frame)
        {
            if (granular_sleep)
            {
                // NOTE:
                // For some reason, the sleep is not accurate enough to wake in time. [HHD018]
                // Thus, I am sleeping 1ms less to make sure we lock at 33.33ms
                DWORD sleep_time = (DWORD) ((1000.0f * (target_seconds_per_frame - secs_elapsed_for_frame)) - 1.0f);
                if (sleep_time > 0)
                {
                    Sleep(sleep_time);
                }
            }

            // NOTE:
            // Should not assert pieces of code that we know WILL happen.
            // Try to only assert when we want to make sure we crash IF the thing happens!!!
            f32 test_secs_elapsed_for_frame = win32_get_seconds_elapsed(current_counter, win32_get_seconds_wallclock());
            if (test_secs_elapsed_for_frame < target_seconds_per_frame)
            {
                // TODO: LOGGING HERE!
                // MISSED SLEEP
            }

            while (secs_elapsed_for_frame < target_seconds_per_frame)
            {
                secs_elapsed_for_frame = win32_get_seconds_elapsed(start_counter, win32_get_seconds_wallclock());
            }
        }
        else
        {
            // TODO: Took too long to process current frame!
            // Logging here
        }

        LARGE_INTEGER end_counter = win32_get_seconds_wallclock();
        f32 ms_per_frame = 1000.0f * win32_get_seconds_elapsed(start_counter, end_counter);
        start_counter = end_counter;

        win32_win_dimensions win_size = win32_get_win_dimensions(window);
// #if NEO_INTERNAL
//         win32_render_debug_display_sync(&g_bm_buffer, arr_count(debug_cursors), debug_cursors,
//                                         &sound_settings, target_seconds_per_frame);
// #endif
        win32_update_win_with_buffer(device_context, &g_bm_buffer, win_size.width,
                                     win_size.height);

        // TODO: SOUND FLIP HERE

        engine_input *temp = new_input;
        new_input = old_input;
        old_input = temp;

#if NEO_INTERNAL
        {
            DWORD write_cursor_;
            DWORD play_cursor_;
            if (SUCCEEDED(g_secondary_buffer->GetCurrentPosition(&play_cursor_,
                                                           &write_cursor_)))
            {
                debug_cursors[debug_cursor_index++] = play_cursor_;
                if (debug_cursor_index == arr_count(debug_cursors))
                {
                    debug_cursor_index = 0;
                }
            }
        }
#endif
#if 0
        i32 fps = 0;
        char perf_record[256];
        _snprintf_s(perf_record, sizeof(perf_record), "MS / frame: %.02fms/f | FPS: %d\n", ms_per_frame, fps);
        OutputDebugStringA(perf_record);
#endif

    }

    return 0;
}
