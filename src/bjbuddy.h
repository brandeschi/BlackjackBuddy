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
  u32 rounds_per_hour;
  u32 unit_amt;
};

enum ace_splitting
{
  ACE_NO_SPLITTING =   1 << 0,
  ACE_NO_RESPLITTING = 1 << 1,
  ACE_RESPLITTING =    1 << 2,
};
struct table_rules
{
  f32 penetration;
  ace_splitting resplit_aces;
  u8 number_of_decks;
  u8 max_hands;
  b8 h17;
  b8 das;
  b8 surrender;
};

enum scene
{
  s_QUIT = 0,
  s_MENU,
  s_GAME,
  s_SIMU,
};

// TODO: Optimize size of game structs.
struct app_state
{
  memory_arena core_arena;
  table_rules table_rules;
  shoe shoe;
  player ap;
  hand dealer;
  phase game_phase;
  scene scene;
  s32 running_count;
  f32 true_count;
  b32 scene_initialized;
};

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input,
                            engine_bitmap_buffer *Buffer);

static void app_get_sound_samples(thread_context *Thread, app_memory *Memory, engine_sound_buffer *SoundBuffer);

// NOTE: Forward Declares
internal void Hit(shoe *Shoe, hand *Hand, s32 *RCount, b32 IsDoubleDown);

