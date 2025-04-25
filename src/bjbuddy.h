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
  engine_controller_input controllers[2];
};

inline engine_controller_input *GetController(engine_input *Input, ums ControllerIndex = 0)
{
  NeoAssert(ArrayCount(Input->controllers) > ControllerIndex);

  engine_controller_input *Result = &Input->controllers[ControllerIndex];
  return Result;
}

enum card_type
{
  // FACE_DOWN,
  ACE = 0,
  TWO,
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
  KING
};

enum suit
{
  SPADES = 1,
  HEARTS,
  DIAMONDS,
  CLUBS
};

enum phase
{
  NULL_PHASE = 0,
  START,
  PLAYER,
  DEALER,
  END
};

struct card
{
  card_type type;
  suit suit;

  u32 value;
  b32 is_dd = false;
};

// NOTE: Technically a deck is 52 cards however, in shoe games
// there are multiple decks and so currenlty I am not sure
// if I should define a deck to have 52 cards or if it should
// just be a ptr and a deck is treated as one big unit.
struct deck
{
  card cards[52];
  card *current;
};

struct hand
{
  card *cards;
  u32 card_count;

  u32 value;
  u32 wager;
};

struct player
{
  u32 bankroll;
  u32 hand_count;
  hand *hands;
};

// TODO: Optimize size of game structs.
struct app_state
{
  memory_arena core_arena;
  deck base_deck;
  hand dealer;
  player ap;

  u32 player_money;
  phase game_phase;

  hand *current_hand;
};

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input,
                            engine_bitmap_buffer *Buffer);

static void app_get_sound_samples(thread_context *Thread, app_memory *Memory, engine_sound_buffer *SoundBuffer);

