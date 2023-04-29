#include "nebula.h"

// TODO: Implment all deck actions
// - Shuffling
// - Drawing Cards
// - Concept of a hand?
//
// Also need to add general player actions
deck base_deck = {
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

// NOTE: CPP is not obligated to pack structs the way we want so sometimes this is necessary
#pragma pack(push, 1) // Push how closely to pack bytes
struct bmp_header
{
    u16 file_type;
    u32 file_size;
    u16 res_1;
    u16 res_2;
    u32 bitmap_offset;
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
};
#pragma pack(pop) // Pop back to how the compiler was packing previously

// TODO: This is not final jpg loading code!
static loaded_jpg DEBUG_load_jpg(thread_context *thread, debug_read_entire_file *read_entire_file, char *file_name)
{
    loaded_jpg result = {};
    debug_file_result read_result = read_entire_file(thread, file_name);
    int x = 0;
    // if (read_result.contents_size != 0)
    // {
    //     bmp_header *header = (bmp_header *)read_result.contents;
    //     u32 *pixels = (u32 *)((u8 *)read_result.contents + header->bitmap_offset);
    //
    //     // NOTE: For some reason I did not have to do any shifting of the bits for my bmp file?
    //     result.pixels = pixels;
    //     result.width = header->width;
    //     result.height = header->height;
    //
    //     assert(header->compression == 3);
    //
    //     // NOTE: Byte order of the bmp pixels in memory is deteremined by the header itself!
    //     u32 alpha_mask = ~(header->red_mask | header->green_mask | header->blue_mask);
    //
    //     bit_scan_result red_shift = find_least_sig_set_bit_32(header->red_mask);
    //     bit_scan_result green_shift = find_least_sig_set_bit_32(header->green_mask);
    //     bit_scan_result blue_shift = find_least_sig_set_bit_32(header->blue_mask);
    //     bit_scan_result alpha_shift = find_least_sig_set_bit_32(alpha_mask);
    //
    //     assert(red_shift.found);
    //     assert(green_shift.found);
    //     assert(blue_shift.found);
    //     assert(alpha_shift.found);
    //
    //     u32 *src_dest = pixels;
    //     for (i32 Y = 0; Y < header->height; Y++)
    //     {
    //         for (i32 X = 0; X < header->width; X++)
    //         {
    //             u32 color = *src_dest;
    //             *src_dest++ = ((((color >> alpha_shift.index) & 0xFF) << 24) |
    //                            (((color >> red_shift.index) & 0xFF) << 16) |
    //                            (((color >> green_shift.index) & 0xFF) << 8) |
    //                            (((color >> blue_shift.index) & 0xFF) << 0)
    //                           );
    //         }
    //     }
    // }

    return result;
}

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

inline i32 round_f32_to_i32(f32 real)
{
    return (i32) (real + 0.5f);
}

inline u32 round_f32_to_u32(f32 real)
{
    return (u32) (real + 0.5f);
}

static void draw_rect(engine_bitmap_buffer *buffer, f32 f_min_X, f32 f_max_X, f32 f_min_Y, f32 f_max_Y,
                      f32 r, f32 g, f32 b)
{
    i32 min_X = round_f32_to_i32(f_min_X);
    i32 max_X = round_f32_to_i32(f_max_X);
    i32 min_Y = round_f32_to_i32(f_min_Y);
    i32 max_Y = round_f32_to_i32(f_max_Y);

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

static void update_and_render(thread_context *thread, app_memory *memory, engine_input *input, engine_bitmap_buffer *bitmap_buffer)
{
    assert(sizeof(app_state) <= memory->perm_storage_space);
    app_state *game_state = (app_state *) memory->perm_mem_storage;

    if(!memory->is_init)
    {
        // game_state->DEBUG_read_entire_file;
        memory->is_init = true;
    }
    #if 0
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

    // Draw debug backgroun in client area.
    draw_rect(bitmap_buffer, 0.0f, (f32)bitmap_buffer->width, 0.0f, (f32)bitmap_buffer->height,
              0.8f, 0.56f, 0.64f);
        #endif
}

