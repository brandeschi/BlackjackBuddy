// TODO: See if I akctually need to tightly pack like this
// #pragma pack(push, 1)
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
    for(u32 i = 0; i < tables_to_add; ++i)
    {
        info->quant_tables[i].precision_index = get_upper_nibble(*(*bytes));
        info->quant_tables[i].id = get_lower_nibble(*(*bytes)++);
        if (info->quant_tables[i].precision_index == 0)
        {
            for (u32 byte = 0; byte < 64; ++byte)
            {
                info->quant_tables[i].table_values[zz_grouping[byte]] = *(*bytes)++;
            }
        }
        else
        {
            for (u32 word = 0; word < 64; ++word, *bytes+=2)
            {
                u16 word_value = *((u16 *)(*bytes));
                info->quant_tables[i].table_values[zz_grouping[word]] = word_value;
            }

        }
    }
}
static void process_dht(memory_arena *ma, u8 **bytes, jpg_info *info)
{
    u16 length = read_next_word((u16 *)(*bytes));
    *bytes += 2;
    while((length - 2) > 0)
    {
        b32 AC_table = get_upper_nibble(*(*bytes));
        u8 table_id = get_lower_nibble(*(*bytes)++);
        --length;
        if(table_id > 3)
        {
            OutputDebugStringA("Too many huff_tables\n");
            return;
        }

        huff_table *ht;
        if(AC_table)
        {
            ht = &info->ac_tables[table_id];
        }
        else
        {
            ht = &info->dc_tables[table_id];
        }
        ht->id = table_id;
        // Read 16 bytes to get size of huff_table, must be <=256
        // NOTE: These 16 bytes tell us the # of values (symbols) per bit index
        // i.e. 00 00 06 02 03 01 00 00 00 00 00 00 00 00 00 00 is
        //      1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 bit(s) also known as code length
        u16 num_of_symbols = 0;
        for(u32 i = 0; i < 16; ++i, --length)
        {
            ht->code_length_count[i] = *(*bytes);
            num_of_symbols += *(*bytes)++;
        }
        ht->num_of_symbols = num_of_symbols;
        for(u32 i = 0; i < num_of_symbols; ++i, --length)
        {
            ht->huff_symbols[i] = *(*bytes)++;
        }
    }
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
    // And that the components are alwayr Y = 1, Cb = 2, Cr = 3
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
        // NOTE: Assuming the components are alwayr Y = 1, Cb = 2, Cr = 3
        info->components[i].huff_DCtable_id = get_upper_nibble(*(*bytes));
        info->components[i].huff_ACtable_id = get_lower_nibble(*(*bytes));
        *bytes += 2;
    }
    // Move bytes up to start of image data
    *bytes += 2;
}

static u32 decode_raw_image_data(u8 **img_data, u8 *huff_data)
{
    // TODO: I am not sure what the best way is with my current memory setup
    // to make a properly sized array in the mem arena. Thus, I am going to
    // store the size that I actually use in the huff_data array.
    u32 result = 0;
    while(true)
    {
        if(*(*img_data) == 0xFF)
        {
            u16 word = read_next_word((u16 *)(*img_data));
            if(word == JPG_EOI)
            {
                return result;
            }
            // Store the FF byte and skip the 00 byte
            else if(word == 0xFF00)
            {
                *huff_data++ = *(*img_data);
                ++result;
                (*img_data) += 2;
            }
            // Ignore Restart markers
            else if((word >= 0xFFD0) && (word <= 0xFFD7))
            {
                (*img_data) += 2;
            }
            // Ignore multi 0xFFs
            else if(word == 0xFF)
            {
                // Move up one byte and test again
                (*img_data)++;
            }
            else
            {
                OutputDebugStringA("Invalid Marker Detected!\n");
                return result;
            }
        }
        else
        {
            // Store bytes that are valid
            *huff_data++ = *(*img_data)++;
            ++result;
        }
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

        // TODO: Need to change this to only parse headers and stop if something goes wrong
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
                        info.restart_inverval_between_mcus = read_next_word((u16 *)bytes);
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
                        info.parsed_huff_data = push_array(ma, image_data_size, u8);
                        info.parsed_huff_data_size = decode_raw_image_data(&bytes, info.parsed_huff_data);
                        // NOTE: At the pixel data now so goto decode img data loop!
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
    _snprintf_s(buff, sizeof(buff), "QT Tables Amt: %d\n\n", info.num_of_qt_tables);
    OutputDebugStringA(buff);
    for(u32 i = 0; i < info.num_of_qt_tables; ++i)
    {
        _snprintf_s(buff, sizeof(buff), "id: %d\n", info.quant_tables[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "precision_index: %d", info.quant_tables[i].precision_index);
        OutputDebugStringA(buff);
        for(u32 j = 0; j < arr_count(info.quant_tables[i].table_values); ++j)
        {
            if (j % 8 == 0)
            {
                OutputDebugStringA("\n");
            }
            _snprintf_s(buff, sizeof(buff), "%d ", info.quant_tables[i].table_values[j]);
            OutputDebugStringA(buff);
        }
        OutputDebugStringA("\n\n");
    }

    // NOTE: Huffman_Tables
    OutputDebugStringA("DC Huffman Tables:\n");
    for(u32 i = 0; i < arr_count(info.dc_tables); ++i)
    {
        if(info.dc_tables[i].num_of_symbols == 0)
        {
            continue;
        }
        _snprintf_s(buff, sizeof(buff), "ID: %d\n", info.dc_tables[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "num_of_symbols: %d\n", info.dc_tables[i].num_of_symbols);
        OutputDebugStringA(buff);
        OutputDebugStringA("Amt per code length: ");
        for(u32 j = 0; j < arr_count(info.dc_tables[i].code_length_count); ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%d ", info.dc_tables[i].code_length_count[j]);
            OutputDebugStringA(buff);
        }
        OutputDebugStringA("\nHuff Symbols: ");
        for(u32 j = 0; j < info.dc_tables[i].num_of_symbols; ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%02X ", info.dc_tables[i].huff_symbols[j]);
            OutputDebugStringA(buff);
        }
        OutputDebugStringA("\n");
    }
    OutputDebugStringA("AC Huffman Tables:\n");
    for(u32 i = 0; i < arr_count(info.ac_tables); ++i)
    {
        if(info.ac_tables[i].num_of_symbols == 0)
        {
            continue;
        }
        _snprintf_s(buff, sizeof(buff), "ID: %d\n", info.ac_tables[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "num_of_symbols: %d\n", info.ac_tables[i].num_of_symbols);
        OutputDebugStringA(buff);
        OutputDebugStringA("Amt per code length: ");
        for(u32 j = 0; j < arr_count(info.ac_tables[i].code_length_count); ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%d ", info.ac_tables[i].code_length_count[j]);
            OutputDebugStringA(buff);
        }
        OutputDebugStringA("\nHuff Symbols: ");
        for(u32 j = 0; j < info.ac_tables[i].num_of_symbols; ++j)
        {
            _snprintf_s(buff, sizeof(buff), "%02X ", info.ac_tables[i].huff_symbols[j]);
            OutputDebugStringA(buff);
        }
        OutputDebugStringA("\n");

    }
    OutputDebugStringA("\n");

    // NOTE: Components Info
    _snprintf_s(buff, sizeof(buff), "Color Space Components Count: %zu\n", arr_count(info.components));
    OutputDebugStringA(buff);
    for(u32 i = 0; i < arr_count(info.components); ++i)
    {
        _snprintf_s(buff, sizeof(buff), "ID: %d\n", info.components[i].id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "QTT_ID: %d\n", info.components[i].qt_table_id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "HDCT_ID: %d\n", info.components[i].huff_DCtable_id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "HACT_ID: %d\n", info.components[i].huff_ACtable_id);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "H_Sampling: %d\n", info.components[i].h_sampling);
        OutputDebugStringA(buff);
        _snprintf_s(buff, sizeof(buff), "V_Sampling: %d\n", info.components[i].v_sampling);
        OutputDebugStringA(buff);
    }
    return result;
}


