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
#if 0
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
#endif
// Decode the Huffman data
// Un-Quantize by multiplying by the Quantization table
// Inverse the DCT
// All of this done for each 8x8 image
#pragma pack(push, 1) // Push how closely to pack bytes
// Exif JPG
struct jpg_header
{
    u16 file_type; // always 0xFFD8
    u16 file_type_marker; // APP1 marker; always 0xFFE1
    u16 file_size;
    u32 res_1; // Exif header part1
    u16 res_2; // Exif header part2
    u16 byte_alignment; // Either "I I" or "MM"                                     //
    u16 tag_mark; // reserved for after alignment                                   // TIFF Header
    u32 first_ifd_offset; // 1st Image File Directory offset (usually 0x00000008)   //


};
#pragma pack(pop) // Pop back to how the compiler was packing previously

#define JPG_SOI (u16)0xFFD8
#define JPG_SOF (u16)0xFFC0
#define JPG_DHT (u16)0xFFC4
#define JPG_DQT (u16)0xFFDB
#define JPG_DRI (u16)0xFFDD
#define JPG_SOS (u16)0xFFDA
#define JPG_EOI (u16)0xFFD9

static u16 endian_swap_word(u16 word)
{
    u16 result = ((word & 0xFF00) >> 8) | ((word & 0x00FF) << 8);
    return result;
}
static u16 read_next_word(u16 *bytes)
{
    u16 result = endian_swap_word(*bytes++);
    return result;
}
// TODO: This is not final jpg loading code!
static loaded_jpg DEBUG_load_jpg(thread_context *thread, debug_read_entire_file *read_entire_file, char *file_name)
{
    loaded_jpg result = {};
    debug_file_result read_result = read_entire_file(thread, file_name);
    if (read_result.contents_size != 0)
    {
        u8 *bytes = (u8 *)read_result.contents;
        // while(read_next_word((u16 *)bytes) != (u16)0xFFD9)
        // {
        //     char test[64];
        //     u16 marker = *(u16 *)bytes;
        //     _snprintf_s(test, sizeof(test), "EOI: %x\n", marker);
        //     OutputDebugStringA(test);
        //     bytes = bytes + 2;
        // }
        if (read_next_word((u16 *)bytes) != JPG_SOI)
        {
            OutputDebugStringA("Not a JPG\n");
            return result;
        }

        bytes = bytes + 2;
        u16 current_word;
        while((current_word = read_next_word((u16 *)bytes)) != (u16)0xFFD9)
        {
            switch(current_word)
            {
                case JPG_SOF:
                {
                    OutputDebugStringA("sof\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } break;
                case JPG_DHT:
                {
                    OutputDebugStringA("dht\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } break;
                case JPG_DQT:
                {
                    OutputDebugStringA("dqt\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } break;
                case JPG_DRI:
                {
                    OutputDebugStringA("dri\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } break;
                case JPG_SOS:
                {
                    OutputDebugStringA("sos\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } goto exit_loop;
                default:
                {
                    OutputDebugStringA("def\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } break;

            }
        }
    exit_loop: ;
    }

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

