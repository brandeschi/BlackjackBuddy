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

// Decode the Huffman data
// Un-Quantize by multiplying by the Quantization table
// Inverse the DCT
// All of this done for each 8x8 image
// Exif JPG
// struct jpg_header
// {
//     u16 file_type; // always 0xFFD8
//     u16 file_type_marker; // APP1 marker; always 0xFFE1
//     u16 file_size;
//     u32 res_1; // Exif header part1
//     u16 res_2; // Exif header part2
//     u16 byte_alignment; // Either "I I" or "MM"                                     //
//     u16 tag_mark; // reserved for after alignment                                   // TIFF Header
//     u32 first_ifd_offset; // 1st Image File Directory offset (usually 0x00000008)   //
//
//
// };

#define JPG_SOI (u16)0xFFD8
#define JPG_SOF (u16)0xFFC0
#define JPG_DHT (u16)0xFFC4
#define JPG_DQT (u16)0xFFDB
#define JPG_DRI (u16)0xFFDD
#define JPG_SOS (u16)0xFFDA
#define JPG_CME (u16)0xFFFE
#define JPG_EOI (u16)0xFFD9

// TODO: See if I akctually need to tightly pack like this
// #pragma pack(push, 1)
struct component_info
{
    u8 id;
    u8 h_sampling;
    u8 v_sampling;
    u8 qt_table_id;
};
struct qt_table
{
    // NOTE: If 0, the table uses bytes... If 1, the table uses words...
    u8 precision_index;
    u8 table_values[64];
};
struct huff_table
{
    // NOTE: Read top nibble; if >0, then it is an AC table else it is a DC table
    u8 table_type;
    u8 *huff_values;
};
// #pragma pack(pop)

// TODO: Get a refresher on how best to order props of a struct
struct jpg_info
{
    qt_table *qt_tables;
    huff_table *huff_tables;
    u32 *pixels;
    u16 bytes_between_mcu;
    b32 grayscale;
    u8 bits_per_sample;
    u16 image_width;
    u16 image_height;
    component_info components[3];
};

inline static u16 endian_swap_word(u16 word)
{
    u16 result = ((word & 0xFF00) >> 8) | ((word & 0x00FF) << 8);
    return result;
}
static u16 read_next_word(u16 *bytes)
{
    u16 result = endian_swap_word(*bytes++);
    return result;
}
inline static u8 get_upper_nibble(u8 byte)
{
    u8 result = (byte >> 4);
    return result;
}
inline static u8 get_lower_nibble(u8 byte)
{
    u8 result = (byte & 0x0F);
    return result;
}

static void process_dqt(memory_arena *ma, u8 *base_address, u16 length_minus_lword, jpg_info *info)
{
    // Move past length word
    base_address = base_address + 2;
    u16 tables_to_add = length_minus_lword / (u16)65;
    while (tables_to_add)
    {
        info->qt_tables = push_struct(ma, qt_table);
        info->qt_tables->precision_index = get_upper_nibble(*base_address++);
        if (info->qt_tables->precision_index == 0)
        {
            for (u8 byte = 0; byte < 64; ++byte, ++base_address)
            {
                info->qt_tables->table_values[byte] = *base_address;
            }
        }
        --tables_to_add;
    }

    // Move our ptr back to the first qt_table
    info->qt_tables = info->qt_tables - tables_to_add;
}
static void process_dht(memory_arena *ma, u8 *bytes, u16 length_wo_lbyte, jpg_info *info)
{
    // Move past length word
    bytes = bytes + 2;
    u32 table_count = 0;
    u32 read_bytes = 0;
    huff_table *temp = 0;
    while (length_wo_lbyte > read_bytes)
    {
        info->huff_tables = push_struct(ma, huff_table);
        // Save beginning of huff_tables
        if (table_count == 0)
        {
            temp = info->huff_tables;
        }
        info->huff_tables->table_type = *bytes++;
        ++read_bytes;

        // Read 16 bytes to get size of huff_table
        u32 huff_size = 0;
        for (u32 i = 0; i < 16; ++i, ++read_bytes)
        {
            huff_size += *bytes++;
        }
        info->huff_tables->huff_values = push_array(ma, huff_size, u8);
        for (u32 i = 0; i < huff_size; ++i, ++read_bytes)
        {
            info->huff_tables->huff_values[i] = *bytes++;
        }
        ++table_count;
    }

    info->huff_tables = temp;
}

static void process_sof(memory_arena *ma, u8 *bytes, u16 length_wo_lbyte, jpg_info *info)
{
    bytes = bytes + 2;
    info->bits_per_sample = *bytes++;
    // Swap endianess to get the proper word value
    info->image_height = read_next_word((u16 *)bytes);
    bytes += 2;
    info->image_width = read_next_word((u16 *)bytes);
    bytes += 2;
    if (*bytes++ == 3)
    {
        info->grayscale = false;
    }
    else
    {
        info->grayscale = true;
    }
    // NOTE: Making an assumption that the jpeg is never grayscale
    for (u32 i = 0; i < 3; ++i)
    {
        info->components[i].id = *bytes++;
        info->components[i].h_sampling = get_upper_nibble(*bytes);
        info->components[i].v_sampling = get_lower_nibble(*bytes++);
        info->components[i].qt_table_id = *bytes++;
    }
}

// TODO: This is not final jpg loading code!
static loaded_jpg DEBUG_load_jpg(memory_arena *ma, thread_context *thread, debug_read_entire_file *read_entire_file, char *file_name)
{
    jpg_info info = {};
    loaded_jpg result = {};
    debug_file_result read_result = read_entire_file(thread, file_name);
    if (read_result.contents_size != 0)
    {
        u8 *bytes = (u8 *)read_result.contents;
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
                    process_sof(ma, bytes, length - 2, &info);
                    bytes = bytes + length;
                } break;
                case JPG_DHT:
                {
                    OutputDebugStringA("dht\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    // char test[512];
                    // _snprintf_s(test, sizeof(test), "ma size before: %zu\n", sizeof(info.qt_tables));
                    // OutputDebugStringA(test);
                    process_dht(ma, bytes, length - 2, &info);
                    bytes = bytes + length;
                } break;
                case JPG_DQT:
                {
                    OutputDebugStringA("dqt\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    process_dqt(ma, bytes, length - 2, &info);
                    bytes = bytes + length;
                } break;
                case JPG_DRI:
                {
                    OutputDebugStringA("dri\n");
                    bytes = bytes + 4;
                    info.bytes_between_mcu = read_next_word((u16 *)bytes);
                    bytes = bytes + 2;
                } break;
                case JPG_SOS:
                {
                    OutputDebugStringA("sos\n");
                    bytes = bytes + 2;
                    u16 length = read_next_word((u16 *)bytes);
                    bytes = bytes + length;
                } goto exit_loop; // Exit out of loop since we are now at the pixels
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

    // TODO: Print out all data from markers to verify?
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

