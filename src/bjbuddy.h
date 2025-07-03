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
  r_Two = 0,
  r_Three,
  r_Four,
  r_Five,
  r_Six,
  r_Seven,
  r_Eight,
  r_Nine,
  r_Ten = 8,
  r_Jack = 8,
  r_Queen = 8,
  r_King = 8,
  r_Ace,
};

enum suit
{
  su_Spades = 1,
  su_Hearts,
  su_Diamonds,
  su_Clubs
};

enum phase
{
  p_Null,
  p_Start,
  p_Player,
  p_Dealer,
  p_Eval,
  p_End
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
  as_NoSplitting = 1,
  as_NoResplitting,
  as_Resplitting,
};
struct table_rules
{
  f32 penetration; // NOTE: As a percentage
  ace_splitting resplit_aces;
  u8 number_of_decks;
  u8 max_hands;
  b8 h17;
  b8 das;
  b8 surrender;
  b8 burn_card;
};

enum scene
{
  sc_Quit = 0,
  sc_Menu,
  sc_Game,
  sc_Simu,
};

enum game_mode
{
  gm_BasicStrategy,
  gm_Counting,
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
  game_mode game_mode;
  s32 running_count;
  f32 true_count;
  b32 scene_initialized;
};

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input,
                            engine_bitmap_buffer *Buffer);

static void app_get_sound_samples(thread_context *Thread, app_memory *Memory, engine_sound_buffer *SoundBuffer);

// NOTE: Forward Declares
internal void Hit(shoe *Shoe, hand *Hand, s32 *RCount, b32 IsDoubleDown);

