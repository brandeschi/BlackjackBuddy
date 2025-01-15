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

inline engine_controller_input *GetController(engine_input *Input, ums ControllerIndex = 0)
{
  // NOTE: might want to make controller_index unsigned if we don't want neg arr access
  NeoAssert(ArrayCount(Input->controllers) > ControllerIndex);

  engine_controller_input *Result = &Input->controllers[ControllerIndex];
  return Result;
}

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
  memory_arena core_arena;
  deck base_deck;
};

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input,
                            engine_bitmap_buffer *Buffer);

static void app_get_sound_samples(thread_context *Thread, app_memory *Memory, engine_sound_buffer *SoundBuffer);

