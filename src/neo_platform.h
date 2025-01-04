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

// TODO: Will need to separate out the bitmap loading, handling, etc
// for other platforms.

// TODO: Factor this out into something that can be platform-independent.
#define DEBUG_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *file)
typedef DEBUG_FREE_FILE_MEMORY(debug_free_file);

#define DEBUG_READ_ENTIRE_FILE(name) debug_file_result name(thread_context *thread, char *file_name)
typedef DEBUG_READ_ENTIRE_FILE(debug_read_entire_file);

#define DEBUG_WRITE_ENTIRE_FILE(name) b32 name(thread_context *thread, char *file_name, void *file, u32 file_size)
typedef DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file);
// #endif

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
  ums size;
  u8 *base_address;
  mem_index used_space;
};

static void InitArena(memory_arena *Arena, ums Size, u8 *BaseAddress)
{
  Arena->size = Size;
  Arena->base_address = BaseAddress;
  Arena->used_space = 0;
}

#define PushStruct(ma, type) (type *)PushSize_(ma, sizeof(type))
#define PushArray(ma, count, type) (type *)PushSize_(ma, (count) * sizeof(type))
void *PushSize_(memory_arena *Arena, ums Size)
{
  NeoAssert((Arena->used_space + Size) <= Arena->size);
  void *Result = Arena->base_address + Arena->used_space;
  Arena->used_space += Size;
  return Result;
}

// NOTE: CPP is not obligated to pack structs the way we want so sometimes this is necessary
#pragma pack(push, 1) // Push how closely to pack bytes
struct bmp_header
{
  u16 file_type;
  u32 file_size;
  u16 res_1;
  u16 res_2;
  u32 bitmap_offset;
  // DIB Header v5
  u32 size;
  s32 width;
  s32 height;
  u16 planes;
  u16 bits_per_pixel;
  u32 compression;
  u32 size_of_bitmap;
  s32 horz_resolution;
  s32 vert_resolution;
  u32 colors_used;
  u32 colors_important;
  u32 red_mask;
  u32 green_mask;
  u32 blue_mask;
  u32 alpha_mask;
  // DWORD        bV5CSType;
  // CIEXYZTRIPLE bV5Endpoints;
  // DWORD        bV5GammaRed;
  // DWORD        bV5GammaGreen;
  // DWORD        bV5GammaBlue;
  // DWORD        bV5Intent;
  // DWORD        bV5ProfileData;
  // DWORD        bV5ProfileSize;
  // DWORD        bV5Reserved;
};
#pragma pack(pop) // Pop back to how the compiler was packing previously

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

