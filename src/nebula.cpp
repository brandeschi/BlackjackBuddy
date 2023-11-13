#include "nebula.h"

// TODO: Implment all deck actions
// - Shuffling
// - Drawing Cards
// - Concept of a hand?
//
// Also need to add general player actions
#include <stdlib.h>
#include <time.h>
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
    i16 tone_volume = 1000;
    int wave_period = sound_buffer->samples_per_second / game_state->tone_hz;
    // int wave_period = sound_buffer->samples_per_second / game_state->tone_hz;

    i16 *sample_out = sound_buffer->samples;
    for(int sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
    {
        float sine_value = sinf(game_state->t_sine);
        i16 sample_value = (i16) (sine_value * tone_volume);
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
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    i32 horz_resolution;
    i32 vert_resolution;
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

        neo_assert(red_shift.found);
        neo_assert(green_shift.found);
        neo_assert(blue_shift.found);
        neo_assert(alpha_shift.found);

        u32 *src_dest = (u32 *)pixels;
        for (i32 Y = 0; Y < header->height; Y++)
        {
            for (i32 X = 0; X < header->width; X++)
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
    i32 min_X = round_f32_to_i32(0);
    i32 max_X = round_f32_to_i32(0 + (f32)bmp->width);
    // i32 max_X = round_f32_to_i32(225.f);
    i32 min_Y = round_f32_to_i32(0);
    i32 max_Y = round_f32_to_i32(0 + (f32)bmp->height);
    // i32 max_Y = round_f32_to_i32(310.f);

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
        for (i32 y = min_Y; y < max_Y; ++y)
        {
            u32 *dest = (u32 *)dest_row;
            u8 *src = src_row;
            for (i32 x = min_X; x < max_X; ++x)
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
        for (i32 y = min_Y; y < max_Y; y++)
        {
            u32 *dest = (u32 *)dest_row;
            u32 *src = src_row;
            for (i32 x = min_X; x < max_X; x++)
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
    i32 min_X = round_f32_to_i32(min.x);
    i32 max_X = round_f32_to_i32(max.x);
    i32 min_Y = round_f32_to_i32(min.y);
    i32 max_Y = round_f32_to_i32(max.y);

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

    u32 color = (u32)((round_f32_to_u32(r * 255.0f) << 16) |
                      (round_f32_to_u32(g * 255.0f) << 8) |
                      (round_f32_to_u32(b * 255.0f) << 0));
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

loaded_bmp *slice_card_atlas(memory_arena *ma, loaded_bmp card_atlas)
{
    i32 card_width = 98 * 4;
    i32 card_height = 158 * 4;
    loaded_bmp *result = push_array(ma, 52, loaded_bmp);
    for (u32 i = 0; i < 52; ++i)
    {
        result[i].channels = card_atlas.channels;
        result[i].width = card_width;
        result[i].height = card_height;
        u32 *current_pixels = (u32 *)card_atlas.pixels;
        result[i].pixels = (u8 *)(current_pixels + ((card_width*(i % 13)) + ((i / 13)*(card_atlas.width*card_height))));
    }

    return result;
}

static void update_and_render(thread_context *thread, app_memory *memory, engine_input *input, engine_bitmap_buffer *bitmap_buffer)
{
    neo_assert(sizeof(app_state) <= memory->perm_storage_space);
    app_state *game_state = (app_state *)memory->perm_mem_storage;

    if(!memory->is_init)
    {
        init_arena(&game_state->gm_arena, memory->perm_storage_space - sizeof(app_state),
                   (u8 *)memory->perm_mem_storage + sizeof(app_state));
        // NOTE: Each card is like 98 hori and 153 vert
        game_state->tex_atlas = DEBUG_load_bmp(thread, memory->DEBUG_read_entire_file, "test/cards.bmp");
        // game_state->tex_atlas = DEBUG_load_bmp(thread, memory->DEBUG_read_entire_file, "test/debug-art.bmp");
        loaded_bmp *card_bmps = slice_card_atlas(&game_state->gm_arena, game_state->tex_atlas);

        game_state->base_deck = {
            {
                { TWO, "SPADES", card_bmps[39] },
                { THREE, "SPADES", card_bmps[40] },
                { FOUR, "SPADES", card_bmps[41] },
                { FIVE, "SPADES", card_bmps[42] },
                { SIX, "SPADES", card_bmps[43] },
                { SEVEN, "SPADES", card_bmps[44] },
                { EIGHT, "SPADES", card_bmps[45] },
                { NINE, "SPADES", card_bmps[46] },
                { TEN, "SPADES", card_bmps[47] },
                { JACK, "SPADES", card_bmps[48] },
                { QUEEN, "SPADES", card_bmps[49] },
                { KING, "SPADES", card_bmps[50] },
                { ACE, "SPADES", card_bmps[51] },
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
                { ACE, "CLUBS", card_bmps[0] },
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

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Setting Texture wrapping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        // Setting Texture filtering methods
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // TODO: GL_BGRA_EXT is a windows specific value, I believe I need to somehow handle this in the platform layer
        // or when I pull this rendering code out
        card first_card = game_state->base_deck.cards[38];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game_state->tex_atlas.width, game_state->tex_atlas.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, game_state->tex_atlas.pixels + (392 * 1));

        memory->is_init = true;
    }
    for (int controller_index = 0;
         controller_index < arr_count(input->controllers);
         controller_index++)
    {
        engine_controller_input *controller = get_controller(input, controller_index);
        if (controller->is_analog)
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

