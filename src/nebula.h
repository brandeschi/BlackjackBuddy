#pragma once

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

// Type Definitions!
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t mem_index;

typedef float f32;
typedef double f64;



// Bytes size maps
#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value) * 1024LL)
#define gigabytes(value) (megabytes(value) * 1024LL)
#define terabytes(value) (gigabytes(value) * 1024LL)

// Beta Assert FIXME
#if NEO_SPEED
#define assert(expression)
#else
#define assert(expression) if (!(expression)) { *(int *)0 = 0; }
#endif

#define invalid_code_path assert(!"InvalidCodePath!");

#define arr_count(array) (sizeof(array) / sizeof((array)[0]))

struct thread_context
{
    int placeholder;
};

inline u32 safe_truncate_int64(u64 value)
{
    assert(value <= 0xFFFFFFFF);
    u32 result = (u32) value;
    return result;
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

struct engine_bitmap_buffer
{
  void *memory;
  int pitch;
  int width;
  int height;
  int bytes_per_pixel;
};

struct engine_sound_buffer
{
  int samples_per_second;
  int sample_count;
  i16 *samples;
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
    i32 mouseX, mouseY, mouseZ;

    f32 time_step_over_frame;
    engine_controller_input controllers[5];
};

inline engine_controller_input *get_controller(engine_input *input, int controller_index)
{
    // NOTE: might want to make controller_index unsigned if we don't want neg arr access
    assert(arr_count(input->controllers) > controller_index);

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
};

enum card_type
{
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
    card_type Value;
    char *Suit;
};

struct deck
{
    card cards[52];
};

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
    assert((ma->used_space + size) <= ma->size);
    void *result = ma->base_address + ma->used_space;
    ma->used_space += size;
    return result;
}

#include "neo_math.h"
#include "math_utils.h"

struct loaded_jpg
{
    u16 width;
    u16 height;
    u8 *pixels;
};

struct app_state
{
    // debug_free_file *DEBUG_free_file;
    // debug_read_entire_file *DEBUG_read_entire_file;
    // debug_write_entire_file *DEBUG_write_entire_file;
};

static void update_and_render(thread_context *thread, app_memory *memory, engine_input *input,
                              engine_bitmap_buffer *buffer);

static void app_get_sound_samples(thread_context *thread, app_memory *memory, engine_sound_buffer *sound_buffer);

