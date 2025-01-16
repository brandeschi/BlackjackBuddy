// TODO: Implment all deck actions
// - Shuffling
// - Drawing Cards
// - Concept of a hand?
//
// Also need to add general player actions
#pragma once
#include "core.unity.h"

// NOTE: This is Fisher-Yates Algo
// TODO: Need to implement random numbers
inline static void shuffle(card deck[], mem_index deck_size)
{
  time_t t;
  srand((unsigned) time(&t));
  for (int i = 0; i < deck_size - 2; i++)
  {
    int j = rand() % (deck_size - 2 - i);
    // Swap
    card temp = deck[i];
    deck[i] = deck[j];
    deck[j] = temp;
  }
}

// NOTE: players includes the dealer
inline static void new_hand_deal(deck *deck, u32 players = 2)
{
  // TODO: Pass the removed cards to the players/board;
  u32 cards_to_remove = players*2;
  // card *cards_removed;
  // for (u32 i = 0; i < cards_to_remove; ++i)
  // {
  //     cards_removed++ = &deck->cards[i];
  // }

  // return cards_removed;
}

#if 0
#include "neo_jpg.h"
#include "neo_jpg.cpp"
#endif

static void output_sound(app_state *game_state, engine_sound_buffer *sound_buffer)
{
#if 0
  s16 tone_volume = 1000;
  int wave_period = sound_buffer->samples_per_second / game_state->tone_hz;
  // int wave_period = sound_buffer->samples_per_second / game_state->tone_hz;

  s16 *sample_out = sound_buffer->samples;
  for(int sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
  {
    float sine_value = sinf(game_state->t_sine);
    s16 sample_value = (s16) (sine_value * tone_volume);
    *sample_out++ = sample_value;
    *sample_out++ = sample_value;

    // NOTE:
    // The reason the sound would pitch up a little if the program ran for 20s was because
    // the sine function would lose it's floating point precision at that time
    // the if fixes this issue!
    game_state->t_sine += 2.0f * pi32 * 1.0f / (float) wave_period;
    if (game_state->t_sine > 2.0f * pi32 * 1.0f)
    {
      game_state->t_sine -= 2.0f * pi32 * 1.0f;
    }
  }

#endif
}

static void app_get_sound_samples(thread_context *thread, app_memory *memory, engine_sound_buffer *sound_buffer)
{
  app_state *game_state = (app_state *) memory->perm_memory;
  // output_sound(game_state, sound_buffer);
}

static void draw_bmp(engine_bitmap_buffer *buffer, loaded_bmp *bmp)
{
  s32 min_X = RoundF32ToS32(0);
  s32 max_X = RoundF32ToS32(0 + (f32)bmp->width);
  // s32 max_X = RoundF32ToS32(225.f);
  s32 min_Y = RoundF32ToS32(0);
  s32 max_Y = RoundF32ToS32(0 + (f32)bmp->height);
  // s32 max_Y = RoundF32ToS32(310.f);

  if(min_X < 0)
  {
    min_X = 0;
  }
  if(max_X > buffer->width)
  {
    max_X = buffer->width;
  }
  if(min_Y < 0)
  {
    min_Y = 0;
  }
  if(max_Y > buffer->height)
  {
    max_Y = buffer->height;
  }

  if(bmp->channels == 3)
  {
    u8 *src_row = bmp->pixels + (bmp->width * (bmp->height - 1))*3;
    u8 *dest_row = (u8 *)buffer->memory + min_X*buffer->bytes_per_pixel + min_Y*buffer->pitch;
    for (s32 y = min_Y; y < max_Y; ++y)
    {
      u32 *dest = (u32 *)dest_row;
      u8 *src = src_row;
      for (s32 x = min_X; x < max_X; ++x)
      {
        *dest++ = (*(src + 2) << 16 | *(src + 1) << 8 | *src);
        src += 3;
      }
      dest_row += buffer->pitch;
      src_row -= (bmp->width*3);
    }
  }
  else
{
    u32 *src_row = ((u32 *)bmp->pixels) + bmp->width * (bmp->height - 1);
    u8 *dest_row = (u8 *)buffer->memory + min_X*buffer->bytes_per_pixel + min_Y*buffer->pitch;
    for (s32 y = min_Y; y < max_Y; y++)
    {
      u32 *dest = (u32 *)dest_row;
      u32 *src = src_row;
      for (s32 x = min_X; x < max_X; x++)
      {
        f32 a = (f32)((*src >> 24) & 0xFF) / 255.0f;
        f32 sr = (f32)((*src >> 16) & 0xFF);
        f32 sg = (f32)((*src >> 8) & 0xFF);
        f32 sb = (f32)((*src >> 0) & 0xFF);

        f32 dr = (f32)((*dest >> 16) & 0xFF);
        f32 dg = (f32)((*dest >> 8) & 0xFF);
        f32 db = (f32)((*dest >> 0) & 0xFF);

        // TODO: Learn about pre-multiplied alpha(not pre-multiplied)
        //
        // NOTE: This uses linear alpha blending!
        f32 r = (1.0f - a) * dr + a * sr;
        f32 g = (1.0f - a) * dg + a * sg;
        f32 b = (1.0f - a) * db + a * sb;

        *dest = (((u32)(r + 0.5f) << 16) |
          ((u32)(g + 0.5f) << 8) |
          ((u32)(b + 0.5f) << 0)
        );
        dest++;
        src++;
      }

      dest_row += buffer->pitch;
      src_row -= bmp->width;
    }
  }
}

static void draw_rect(engine_bitmap_buffer *buffer, v2 min, v2 max,
                      f32 r, f32 g, f32 b)
{
  s32 min_X = RoundF32ToS32(min.x);
  s32 max_X = RoundF32ToS32(max.x);
  s32 min_Y = RoundF32ToS32(min.y);
  s32 max_Y = RoundF32ToS32(max.y);

  // Will clamp the values inside of the bitmap_buffer
  if(min_X < 0)
  {
    min_X = 0;
  }
  if(max_X > buffer->width)
  {
    max_X = buffer->width;
  }
  if(min_Y < 0)
  {
    min_Y = 0;
  }
  if(max_Y > buffer->height)
  {
    max_Y = buffer->height;
  }

  u32 color = (u32)((RoundF32ToU32(r * 255.0f) << 16) |
                    (RoundF32ToU32(g * 255.0f) << 8) |
                    (RoundF32ToU32(b * 255.0f) << 0));
  u8 *current_pos = (u8 *)buffer->memory + min_X*buffer->bytes_per_pixel + min_Y*buffer->pitch;

  for (int rows = min_Y; rows < max_Y; rows++)
  {
    u32 *pixel = (u32 *)current_pos;
    for (int cols = min_X; cols < max_X; cols++)
    {
      *pixel++ = color;
    }

    current_pos += buffer->pitch;
  }
}

loaded_bmp *SliceCardAtlas(memory_arena *Arena, loaded_bmp CardAtlas)
{
  // TODO: no magic nums
  s32 CardWidth = 98 * 4;
  s32 CardHeight = 158 * 4;
  loaded_bmp *Result = PushArray(Arena, 52, loaded_bmp);
  for (ums Index = 0; Index < 52; ++Index)
  {
    Result[Index].channels = CardAtlas.channels;
    Result[Index].width = CardWidth;
    Result[Index].height = CardHeight;
    u32 *CurrentPixels = (u32 *)CardAtlas.pixels;
    Result[Index].pixels = (u8 *)(CurrentPixels + ((CardWidth*(Index % 13)) + ((Index / 13)*(CardAtlas.width*CardHeight))));
  }

  return Result;
}

static void DrawCard(vertex_data *VertexArray, loaded_bmp TexAtlas, v2 CardIndex) {
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;

  v2 ComputedTexCoords[] = {
    {(CardIndex.x * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height },
    {(CardIndex.x * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height }
  };
  for (u32 Index = 0; Index < 4; ++Index) {
    VertexArray[Index].tex_coords = ComputedTexCoords[Index];
  }
}

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, renderer *Renderer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_size);
  app_state *GameState = (app_state *)Memory->perm_memory;

  if(!Memory->is_init)
  {
    InitArena(&GameState->core_arena, Memory->perm_storage_size - sizeof(app_state),
              (u8 *)Memory->perm_memory + sizeof(app_state));

    GameState->base_deck = {
      {
        { TWO, "SPADES" },
        { THREE, "SPADES" },
        { FOUR, "SPADES" },
        { FIVE, "SPADES" },
        { SIX, "SPADES" },
        { SEVEN, "SPADES" },
        { EIGHT, "SPADES" },
        { NINE, "SPADES" },
        { TEN, "SPADES" },
        { JACK, "SPADES" },
        { QUEEN, "SPADES" },
        { KING, "SPADES" },
        { ACE, "SPADES" },
        { TWO, "HEARTS" },
        { THREE, "HEARTS" },
        { FOUR, "HEARTS" },
        { FIVE, "HEARTS" },
        { SIX, "HEARTS" },
        { SEVEN, "HEARTS" },
        { EIGHT, "HEARTS" },
        { NINE, "HEARTS" },
        { TEN, "HEARTS" },
        { JACK, "HEARTS" },
        { QUEEN, "HEARTS" },
        { KING, "HEARTS" },
        { ACE, "HEARTS" },
        { TWO, "CLUBS" },
        { THREE, "CLUBS" },
        { FOUR, "CLUBS" },
        { FIVE, "CLUBS" },
        { SIX, "CLUBS" },
        { SEVEN, "CLUBS" },
        { EIGHT, "CLUBS" },
        { NINE, "CLUBS" },
        { TEN, "CLUBS" },
        { JACK, "CLUBS" },
        { QUEEN, "CLUBS" },
        { KING, "CLUBS" },
        { ACE, "CLUBS" },
        { TWO, "DIAMONDS" },
        { THREE, "DIAMONDS" },
        { FOUR, "DIAMONDS" },
        { FIVE, "DIAMONDS" },
        { SIX, "DIAMONDS" },
        { SEVEN, "DIAMONDS" },
        { EIGHT, "DIAMONDS" },
        { NINE, "DIAMONDS" },
        { TEN, "DIAMONDS" },
        { JACK, "DIAMONDS" },
        { QUEEN, "DIAMONDS" },
        { KING, "DIAMONDS" },
        { ACE, "DIAMONDS" }
      }
    };

    // TODO: This is temp... probably want to not do this...
    render_unit *Unit = (render_unit *)Renderer->units;
    vertex_data *Vertices = Unit->vertices;
    // TODO: Update how DrawCard works since we now have the render units.
    // Need to ideally add batching or something as I do not see a reason for
    // why I should not just stuff all the data for a card into one unit.
    // This makes me think I should catergorize each unit so that way at the end,
    // I can stuff all the data for units of the same kind into their respective glBufferSubData
    // calls and then execute the draw call for each type of unit.
    DrawCard(Vertices, Renderer->tex_atlas, {0.0f, 4.0f});
    DrawCard(&Vertices[4], Renderer->tex_atlas, {0.0f, 4.0f});

    Memory->is_init = true;
  }

  for (ums ControllerIndex = 0;
  ControllerIndex < ArrayCount(Input->controllers);
  ControllerIndex++)
  {
    engine_controller_input *Controller = GetController(Input, ControllerIndex);
    if (Controller->is_analog)
    {
    }
    else
    {
    }
  }

}

#if 0
static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, engine_bitmap_buffer *BitmapBuffer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_space);
  app_state *GameState = (app_state *)Memory->perm_mem_storage;

  if(!Memory->is_init)
  {
    InitArena(&GameState->arena, Memory->perm_storage_space - sizeof(app_state),
              (u8 *)Memory->perm_mem_storage + sizeof(app_state));

    Memory->is_init = true;
  }

  // Draw debug backgroun in client area.
  v2 Min = {};
  v2 Max = { (f32)BitmapBuffer->width, (f32)BitmapBuffer->height };
  draw_rect(BitmapBuffer, Min, Max, 0.8f, 0.56f, 0.64f);
  loaded_bmp StubBM = {0};
  draw_bmp(BitmapBuffer, &StubBM);
}
#endif
