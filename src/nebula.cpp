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
#define JPG_APP1 (u16)0xFFE1
#define JPG_APP13 (u16)0xFFED
#define JPG_APP14 (u16)0xFFEE
#define JPG_SOS (u16)0xFFDA
#define JPG_CME (u16)0xFFFE
#define JPG_EOI (u16)0xFFD9

// TODO: See if I akctually need to tightly pack like this
// #pragma pack(push, 1)
struct component_info
{
    u8 id;
    u8 qt_table_id;
    u8 huff_table_id;
    u8 h_sampling;
    u8 v_sampling;
};
struct qt_table
{
    // NOTE: If 0, the table uses bytes... If 1, the table uses words...
    u8 precision_index;
    u8 id;
    u8 table_values[64];
};
struct huff_table
{
    // NOTE: Read top nibble; if >0, then it is an AC table else it is a DC table
    u8 table_type;
    u16 huff_size;
    u8 code_length_count[16];
    u8 huff_values[256];
};
// #pragma pack(pop)

// TODO: Get a refresher on how best to order props of a struct
struct jpg_info
{
    qt_table *qt_tables;
    huff_table *huff_tables;
    u32 *pixels;
    u8 *raw_img_data;
    u16 bytes_between_mcu;
    b32 grayscale;
    u8 num_of_qt_tables;
    u8 num_of_huff_tables;
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

static void process_dqt(memory_arena *ma, u8 **bytes, jpg_info *info)
{
    u16 length = read_next_word((u16 *)(*bytes));
    *bytes += 2;
    u16 tables_to_add = length / (u16)65;
    info->num_of_qt_tables = (u8)tables_to_add;
    while (tables_to_add)
    {
        info->qt_tables = push_struct(ma, qt_table);
        info->qt_tables->precision_index = get_upper_nibble(*(*bytes));
        info->qt_tables->id = get_lower_nibble(*(*bytes)++);
        if (info->qt_tables->precision_index == 0)
        {
            for (u8 byte = 0; byte < 64; ++byte)
            {
                info->qt_tables->table_values[byte] = *(*bytes)++;
            }
        }
        --tables_to_add;
    }

    // Move our ptr back to the first qt_table
    info->qt_tables = info->qt_tables - (info->num_of_qt_tables - 1);
}
static void process_dht(memory_arena *ma, u8 **bytes, jpg_info *info)
{
    u16 length = read_next_word((u16 *)(*bytes));
    *bytes += 2;
    u8 table_count = 0;
    u16 read_bytes = 0;
    huff_table *temp = 0;
    while ((length - 2) > read_bytes)
    {
        info->huff_tables = push_struct(ma, huff_table);
        // Save beginning of huff_tables
        if (table_count == 0)
        {
            temp = info->huff_tables;
        }
        info->huff_tables->table_type = *(*bytes)++;
        ++read_bytes;

        // Read 16 bytes to get size of huff_table, must be <=256
        // NOTE: These 16 bytes tell us the # of values (symbols) per bit index
        // i.e. 00 00 06 02 03 01 00 00 00 00 00 00 00 00 00 00 is
        //      1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 bit(s) also known as code length
        u16 huff_size = 0;
        for (u32 i = 0; i < 16; ++i, ++read_bytes)
        {
            info->huff_tables->code_length_count[i] = *(*bytes);
            huff_size += *(*bytes)++;
        }
        info->huff_tables->huff_size = huff_size;
        for (u32 i = 0; i < huff_size; ++i, ++read_bytes)
        {
            info->huff_tables->huff_values[i] = *(*bytes)++;
        }
        ++table_count;
    }
    info->num_of_huff_tables = table_count;
    info->huff_tables = temp;
}

static void process_sof(memory_arena *ma, u8 **bytes, jpg_info *info)
{
    // Move over length since it is not needed
    *bytes += 2;
    info->bits_per_sample = *(*bytes)++;
    // Swap endianess to get the proper word value
    info->image_height = read_next_word((u16 *)(*bytes));
    *bytes += 2;
    info->image_width = read_next_word((u16 *)(*bytes));
    *bytes += 2;
    if (*(*bytes)++ == 3)
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
        info->components[i].id = *(*bytes)++;
        info->components[i].h_sampling = get_upper_nibble(*(*bytes));
        info->components[i].v_sampling = get_lower_nibble(*(*bytes)++);
        info->components[i].qt_table_id = *(*bytes)++;
    }
}

static void process_sos(memory_arena *ma, u8 **bytes, jpg_info *info)
{
    *bytes += 2;
    u8 num_of_components = *(*bytes);
    *bytes += 2;
    for (u32 i = 0; i < num_of_components; ++i)
    {
        info->components[i].huff_table_id = *(*bytes);
        *bytes += 2;
    }
    // Move bytes up to start of image data
    *bytes += 2;
}

static void decode_raw_image_data(u8 *img_data)
{
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

        while((bytes - (u8 *)read_result.contents) < read_result.contents_size)
        {
            if(*bytes == 0xFF)
            {
                u16 marker = read_next_word((u16 *)bytes);
                switch(marker)
                {
                    case JPG_APP1:
                    case JPG_APP13:
                    case JPG_APP14:
                    {
                        bytes = bytes + 2;
                        bytes = bytes + read_next_word((u16 *)bytes);
                    } break;
                    case JPG_SOF:
                    {
                        OutputDebugStringA("sof\n");
                        bytes = bytes + 2;
                        process_sof(ma, &bytes, &info);
                    } break;
                    case JPG_DHT:
                    {
                        OutputDebugStringA("dht\n");
                        bytes = bytes + 2;
                        // char test[512];
                        // _snprintf_s(test, sizeof(test), "ma size before: %zu\n", sizeof(info.qt_tables));
                        // OutputDebugStringA(test);
                        process_dht(ma, &bytes, &info);
                    } break;
                    case JPG_DQT:
                    {
                        OutputDebugStringA("dqt\n");
                        bytes = bytes + 2;
                        process_dqt(ma, &bytes, &info);
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
                        process_sos(ma, &bytes, &info);
                        u8 *ba_image_data = bytes;
                        u8 *ba_file = (u8 *)read_result.contents;
                        u32 image_data_size = (u32)(read_result.contents_size - (ba_image_data - ba_file));
                        info.raw_img_data = push_array(ma, image_data_size, u8);
                        // decode_raw_image_data(info.raw_img_data);
                        // Read byte by byte for 0xFF?
                        // NOTE: At the pixel data now soo actually perform the decode!
                    } break;
                    case JPG_EOI:
                    {
                        OutputDebugStringA("end\n");
                        bytes = bytes + 2;
                    } break;
                    default:
                    {
                        ++bytes;
                    } break;
                }
            }
            else
            {
                ++bytes;
            }
        }
    }

    char buff[512];
    // NOTE: QT_Tables
    _snprintf_s(buff, sizeof(buff), "QT Tables Amt: %d\n", info.num_of_qt_tables);
    OutputDebugStringA(buff);
    for(u32 i = 0; i < info.num_of_qt_tables; ++i)
    {
        _snprintf_s(buff, sizeof(buff), "id: %d\n", info.qt_tables[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "precision_index: %d\n", info.qt_tables[i].precision_index);
        OutputDebugStringA(buff);
        for(u32 j = 0; j < arr_count(info.qt_tables[i].table_values); ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%d ", info.qt_tables[i].table_values[j]);
            OutputDebugStringA(buff);
        }
        _snprintf_s(buff, sizeof(buff), "\n");
        OutputDebugStringA(buff);
    }
    _snprintf_s(buff, sizeof(buff), "\n");
    OutputDebugStringA(buff);

    // NOTE: Huffman_Tables
    _snprintf_s(buff, sizeof(buff), "Huffman Tables Amt: %d\n", info.num_of_huff_tables);
    OutputDebugStringA(buff);
    for(u32 i = 0; i < info.num_of_huff_tables; ++i)
    {
        _snprintf_s(buff, sizeof(buff), "table type: %02X\n", info.huff_tables[i].table_type);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "huff_size: %d\n", info.huff_tables[i].huff_size);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "Amt per code length: ");
        OutputDebugStringA(buff);
        for(u32 j = 0; j < arr_count(info.huff_tables[i].code_length_count); ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%d ", info.huff_tables[i].code_length_count[j]);
            OutputDebugStringA(buff);
        }
        _snprintf_s(buff, sizeof(buff), "\nHuff Values: ");
        OutputDebugStringA(buff);
        for(u32 j = 0; j < info.huff_tables[i].huff_size; ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%d ", info.huff_tables[i].huff_values[j]);
            OutputDebugStringA(buff);
        }
        _snprintf_s(buff, sizeof(buff), "\n");
        OutputDebugStringA(buff);
    }
    _snprintf_s(buff, sizeof(buff), "\n");
    OutputDebugStringA(buff);

    // NOTE: Components Info
    _snprintf_s(buff, sizeof(buff), "Color Space Components Count: %zu\n", arr_count(info.components));
    OutputDebugStringA(buff);
    for(u32 i = 0; i < arr_count(info.components); ++i)
    {
        _snprintf_s(buff, sizeof(buff), "ID: %d\n", info.components[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "QTT_ID: %d\n", info.components[i].qt_table_id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "HT_ID: %02X\n", info.components[i].huff_table_id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "H_Sampling: %d\n", info.components[i].h_sampling);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "V_Sampling: %d\n", info.components[i].v_sampling);
        OutputDebugStringA(buff);
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

