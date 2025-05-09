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

enum rank
{
  TWO = 0,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN = 8,
  JACK = 8,
  QUEEN = 8,
  KING = 8,
  ACE,
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
  EVAL,
  END
};

struct card
{
  rank rank;
  suit suit;
  // TODO: Possible get rid of this later.
  v2 card_coords;

  u32 value;
  b32 is_dd = false;
};

struct shoe
{
  card *cards;
  card *current;
  u32 card_count;
  u32 discarded;
  u32 cut_card;
  u32 deck_count;
};

struct hand
{
  card *cards;
  u32 card_count;

  u32 value;
  f32 wager;
};

struct player
{
  hand *hands;
  ums hand_idx;
  u32 hand_count;
  f32 bankroll;
};

enum table_rules
{
  S17 = 0,
  H17 = 1 << 0,
};

enum scene
{
  GAME = 0,
  SIM  = 1 << 0,
};

// TODO: Optimize size of game structs.
struct app_state
{
  memory_arena core_arena;
  shoe shoe;
  hand dealer;
  player ap;
  phase game_phase;
  s32 running_count;
  f32 true_count;
  s32 table_rules;
  s32 scene;
};

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input,
                            engine_bitmap_buffer *Buffer);

static void app_get_sound_samples(thread_context *Thread, app_memory *Memory, engine_sound_buffer *SoundBuffer);

// NOTE: Forward Declares
internal void Hit(shoe *Shoe, hand *Hand, s32 *RCount, b32 IsDoubleDown);

