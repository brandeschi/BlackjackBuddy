/*
 * NOTE:
 *
 * NEO_SPEED:
 * 0 = slow code (asserts, debug stuff, etc)
 * 1 = Quick code only!
 *
 * NEO_INTERNAL
 * 0 = Release Build
 * 1 = Develop Build
 *
 */

// Bytes size maps
#define kilobytes(Value) ((Value)*1024LL)
#define megabytes(Value) (kilobytes(Value) * 1024LL)
#define gigabytes(Value) (megabytes(Value) * 1024LL)
#define terabytes(Value) (gigabytes(Value) * 1024LL)

#define KEEB_COUNT 1

#if NEO_SPEED
#define NeoAssert(expression)
#endif

struct thread_context
{
    int placeholder;
};

inline u32 SafeTruncateU64(u64 Value)
{
    NeoAssert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return Result;
}

// #if NEO_INTERNAL
struct debug_file_result
{
    u32 contents_size;
    void *contents;
};

#define DEBUG_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *file)
typedef DEBUG_FREE_FILE_MEMORY(debug_free_file);

#define DEBUG_READ_ENTIRE_FILE(name) debug_file_result name(thread_context *thread, char *file_name)
typedef DEBUG_READ_ENTIRE_FILE(debug_read_entire_file);

#define DEBUG_WRITE_ENTIRE_FILE(name) b32 name(thread_context *thread, char *file_name, void *file, u32 file_size)
typedef DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file);
// #endif

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
           u32 file_size32 = SafeTruncateU64(file_size.QuadPart);
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

struct engine_bitmap_buffer
{
  void *memory;
  s32 pitch;
  s32 width;
  s32 height;
  s32 bytes_per_pixel;
};

struct engine_sound_buffer
{
  int samples_per_second;
  int sample_count;
  s16 *samples;
};

struct engine_button_state
{
  int half_transitions;
  b32 is_down;
};

struct engine_controller_input
{
  b32 is_analog = false;
  float stick_avg_x;
  float stick_avg_y;

  union
  {
    // NOTE: MAKE SURE OUR BUTTONS COUNT MATCHES THE NUMBER OF BUTTONS IN STRUCT!!!
    engine_button_state buttons[12];
    struct
    {
      engine_button_state move_down;
      engine_button_state move_left;
      engine_button_state move_up;
      engine_button_state move_right;

      engine_button_state action_down;
      engine_button_state action_left;
      engine_button_state action_up;
      engine_button_state action_right;

      engine_button_state left_shoulder;
      engine_button_state right_shoulder;

      engine_button_state start;
      engine_button_state back;
    };
  };
};

struct engine_input
{
    engine_button_state mouse_buttons [3];
    s32 mouseX, mouseY, mouseZ;

    f32 time_step_over_frame;
    engine_controller_input controllers[5];
};

inline engine_controller_input *get_controller(engine_input *input, int controller_index)
{
    // NOTE: might want to make controller_index unsigned if we don't want neg arr access
    NeoAssert(ArrayCount(input->controllers) > controller_index);

    engine_controller_input *result = &input->controllers[controller_index];
    return result;
}

struct app_memory
{
    b32 is_init;

    u64 perm_storage_space;
    void *perm_mem_storage; // NOTE: This needs to be cleared to 0 when allocated at startup

    u64 flex_storage_space;
    void *flex_mem_storage;

    debug_free_file *DEBUG_free_file;
    debug_read_entire_file *DEBUG_read_entire_file;
    debug_write_entire_file *DEBUG_write_entire_file;
};

// TODO: Make neo file for arenas
struct memory_arena
{
    mem_index size;
    u8 *base_address;
    mem_index used_space;
};

static void init_arena(memory_arena *ma, mem_index size, u8 *base_address)
{
    ma->size = size;
    ma->base_address = base_address;
    ma->used_space = 0;
}

#define push_struct(ma, type) (type *)push_size_(ma, sizeof(type))
#define push_array(ma, count, type) (type *)push_size_(ma, (count) * sizeof(type))
void *push_size_(memory_arena *ma, mem_index size)
{
    NeoAssert((ma->used_space + size) <= ma->size);
    void *result = ma->base_address + ma->used_space;
    ma->used_space += size;
    return result;
}

struct loaded_jpg
{
    u8 *pixels;
    s32 channels;
    s32 width;
    s32 height;
};
struct loaded_bmp
{
    u8 *pixels;
    s32 channels;
    u32 width;
    u32 height;
};

enum card_type
{
    FACE_DOWN = 0,
    TWO = 2,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK = 10,
    QUEEN = 10,
    KING = 10,
    ACE
};

struct card
{
    card_type value;
    char *suit;
};

struct deck
{
    card cards[52];
};

struct app_state
{
    memory_arena gm_arena;
    deck base_deck;
    loaded_bmp tex_atlas;
};

static void update_and_render(thread_context *thread, app_memory *memory, engine_input *input,
                              engine_bitmap_buffer *buffer);

static void app_get_sound_samples(thread_context *thread, app_memory *memory, engine_sound_buffer *sound_buffer);

