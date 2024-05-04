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
  neo_assert((ma->used_space + size) <= ma->size);
  void *result = ma->base_address + ma->used_space;
  ma->used_space += size;
  return result;
}


struct loaded_jpg
{
  u8 *pixels;
  i32 channels;
  i32 width;
  i32 height;
};
struct loaded_bmp
{
  u8 *pixels;
  i32 channels;
  u32 width;
  u32 height;
};

/*
  * NOTE: Experiment with using the following card encoding:
  *
  * 6-bits to encode a deck where:
  * top two bits represent suit -> 00 = club, 01 = spade, 10 = diamond, 11 = heart
  *
  * last 4 bits are card values
  * 0000 = facedown, 0001 = 2, 0010 = 3, etc.
*/
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
  JACK,
  QUEEN,
  KING,
  ACE
};

enum suit_type
{
  CLUBS =    0x00,
  SPADES =   0x10,
  DIAMONDS = 0x20,
  HEARTS =   0x30
};

enum turn_phase
{
  BETTING = 0,
  PLAYER_ACTION,
  DEALER_ACTION,
  PAYOUT
};

struct deck
{
  u8 cards[52];
  u8 *cursor;
};

struct player
{
  u32 money_amount;
  b32 active_bet = false;
  b32 hit = false;

  memory_arena hand_arena;
};

struct app_state
{
  memory_arena gm_arena;
  loaded_bmp tex_atlas;

  turn_phase current_phase;
  deck base_deck;
  b32 first;

  player *players;
};

static void update_and_render(thread_context *thread, app_memory *memory, engine_input *input,
                              engine_bitmap_buffer *buffer);

static void app_get_sound_samples(thread_context *thread, app_memory *memory, engine_sound_buffer *sound_buffer);

