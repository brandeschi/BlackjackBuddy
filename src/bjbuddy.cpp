// TODO: Implment all deck actions
// - Shuffling
// - Drawing Cards
// - Concept of a hand?
//
// Also need to add general player actions
#pragma once
#include "core.unity.h"

// Globals
global thread_context g_ThreadContext = {};
global GLuint g_ShaderProgram;

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
  app_state *game_state = (app_state *) memory->perm_mem_storage;
  // output_sound(game_state, sound_buffer);
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

// TODO: This is not final bmp loading code!
static loaded_bmp DEBUG_load_bmp(thread_context *thread, debug_read_entire_file *read_entire_file, char *file_name)
{
  loaded_bmp result = {};
  debug_file_result read_result = read_entire_file(thread, file_name);
  if (read_result.contents_size != 0)
  {
    bmp_header *header = (bmp_header *)read_result.contents;
    u8 *pixels = ((u8 *)read_result.contents + header->bitmap_offset);
    u32 *test_pix = (u32 *)pixels;


    result.pixels = pixels;
    result.channels = header->bits_per_pixel / 8;
    result.width = header->width;
    result.height = header->height;

    // NOTE: For some reason I did not have to do any shifting of the bits for my bmp file?
    // If I always take images and export them through gimp, I should be good.
#if 0
    // NOTE: Byte order of the bmp pixels in memory is deteremined by the header itself!
    u32 alpha_mask = ~(header->red_mask | header->green_mask | header->blue_mask);

    bit_scan_result red_shift = find_least_sig_set_bit_32(header->red_mask);
    bit_scan_result green_shift = find_least_sig_set_bit_32(header->green_mask);
    bit_scan_result blue_shift = find_least_sig_set_bit_32(header->blue_mask);
    bit_scan_result alpha_shift = find_least_sig_set_bit_32(alpha_mask);

    NeoAssert(red_shift.found);
    NeoAssert(green_shift.found);
    NeoAssert(blue_shift.found);
    NeoAssert(alpha_shift.found);

    u32 *src_dest = (u32 *)pixels;
    for (s32 Y = 0; Y < header->height; Y++)
    {
      for (s32 X = 0; X < header->width; X++)
      {
        u32 color = *src_dest;
        *src_dest++ = ((((color >> alpha_shift.index) & 0xFF) << 24) |
          (((color >> red_shift.index) & 0xFF) << 16) |
          (((color >> green_shift.index) & 0xFF) << 8) |
          (((color >> blue_shift.index) & 0xFF) << 0)
        );
      }
    }
#endif
  }


  return result;
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

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, engine_bitmap_buffer *BitmapBuffer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_space);
  app_state *GameState = (app_state *)Memory->perm_mem_storage;
  static renderer Renderer;

  if(!Memory->is_init)
  {
    InitArena(&GameState->arena, Memory->perm_storage_space - sizeof(app_state),
              (u8 *)Memory->perm_mem_storage + sizeof(app_state));

    InitRenderer(&Renderer, Thread, Memory);
    // loaded_bmp *card_bmps = slice_card_atlas(&game_state->gm_arena, game_state->tex_atlas);

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

    // TODO: Finish converting over opengl calls from win32 plat_layer

    // Create Shader Program
    g_ShaderProgram = create_ogl_shader_program(g_ThreadContext, "..\\vert.glsl", "..\\frag.glsl");
    // Vertex Data
    // v3 vertices[] = {
    //     { -0.5f, -0.5f, 0.0f }, // V1 pos data
    //     { 1.0f, 0.0f, 0.0f }, // V1 color data
    //     { 0.5f, -0.5f, 0.0f }, // V2 pos data
    //     { 0.0f, 1.0f, 0.0f }, // V2 color data
    //     { 0.0f, 0.5f, 0.0f }, // V3 pos data
    //     { 0.0f, 0.0f, 1.0f }, // V3 color data
    // };
    //
    // v2 tex_coords[] = {
    //     { 0.0f, 0.0f },
    //     { 1.0f, 0.0f },
    //     { 0.5f, 1.0f },
    // };
    //
    // v3 vertices[] = {
    //     { 0.5f, 0.5f, 0.0f },   // top-right
    //     { 0.5f, -0.5f, 0.0f },  // bottom-right
    //     { -0.5f, -0.5f, 0.0f }, // bottom-left
    //     { -0.5f, 0.5f, 0.0f },  // top-left
    // };

    vertex_data Vertices[] = {
      //  pos                     color               tex-coords
      { {100.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
      { {150.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
      { {150.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
      { {100.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left

      { {200.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
      { {250.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
      { {250.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
      { {200.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
    };
    u32 Indices[] = {
      0, 1, 3,    // T1
      1, 2, 3,     // T2

      4, 5, 7,    // T3
      5, 6, 7     // T4
    };

    DrawCard(Vertices, Renderer.tex_atlas, {12.0f, 4.0f});
    DrawCard(&Vertices[4], Renderer.tex_atlas, {11.0f, 4.0f});
    // NOTE: I believe this all should move into a InitRenderer func
    //
    // Create a (V)ertex (B)uffer (O)bject and (V)ertex (A)rray (O)bject
    u32 VAO;
    u32 VBO;
    u32 EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VAO, the bind and set VBOs, then config vertex attribs
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    // Tell opengl how to interpret our vertex data by setting pointers to the attribs
    // pos attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    // color attrib
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(3*sizeof(f32)));
    glEnableVertexAttribArray(1);
    // tex coord attrib
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(6*sizeof(f32)));
    glEnableVertexAttribArray(2);


    GLuint Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Setting Texture wrapping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Setting Texture filtering methods
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TODO: GL_BGRA_EXT is a windows specific value, I believe I need to somehow handle this in the platform layer
    // or when I pull this rendering code out
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Renderer.tex_atlas.width, Renderer.tex_atlas.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Renderer.tex_atlas.pixels);

    // NOTE: Start here!
    mat4 Projection = Mat4Ortho(0.0f, (f32)BitmapBuffer->width, 0.0f, (f32)BitmapBuffer->height, -1.0f, 1.0f);
    mat4 MVP = Projection*Mat4Iden()*Mat4Iden();

    glUseProgram(g_ShaderProgram);
    GLint u_MvpId = glGetUniformLocation(g_ShaderProgram, "u_MVP");
    glUniformMatrix4fv(u_MvpId, 1, GL_FALSE, (f32 *)MVP.e);
    glUseProgram(0);

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

#if 0
  // Draw debug backgroun in client area.
  v2 min = {};
  v2 max = { (f32)bitmap_buffer->width, (f32)bitmap_buffer->height };
  draw_rect(bitmap_buffer, min, max,
            0.8f, 0.56f, 0.64f);
  draw_bmp(bitmap_buffer, &game_state->bg);
#endif
}

