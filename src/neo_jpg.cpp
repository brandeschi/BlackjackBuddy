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

static u32 store_raw_huff_data(u8 **img_data, u8 *huff_data)
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
            else if(word == 0xFFFF)
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

static inline i32 read_bit(jpg_img_data *img_data)
{
    if(img_data->bytes_read >= img_data->data_size)
    {
        return -1;
    }
    i32 result = (*img_data->data >> (7 - img_data->current_bit++)) & 1;
    if(img_data->current_bit == 8)
    {
        ++img_data->data;
        ++img_data->bytes_read;
        img_data->current_bit = 0;
    }
    return result;
}

static inline i32 read_bits(jpg_img_data *img_data, u8 length)
{
    i32 result = 0;
    for(u32 i = 0; i < length; ++i)
    {
        i32 bit = read_bit(img_data);
        if(bit == -1)
        {
            OutputDebugStringA("At the end of img_data\n");
            return -1;
        }
        result = (result << 1) | bit;
    }

    return result;
}

static inline void align_bytes(jpg_img_data *img_data)
{
    if(img_data->bytes_read >= img_data->data_size)
    {
        return;
    }
    if(img_data->current_bit != 0)
    {
        img_data->current_bit = 0;
        ++img_data->data;
    }
}

// NOTE: I believe this is what would 'traverse' the huffman b-tree
static void gen_huff_codes(huff_table *ht)
{
    u32 code = 0;
    u32 idx_in_codes = 0;
    for(u32 i = 0; i < 16; ++i)
    {
        for(u32 j = 0; j < ht->code_length_count[i]; ++j)
        {
            ht->huff_codes[idx_in_codes++] = code++;
        }
        code <<= 1;
    }
}

static u8 get_next_symbol(jpg_img_data *img_data, huff_table *ht)
{
    u32 code = 0;
    u32 idx_in_codes = 0;
    for(u32 i = 0; i < 16; ++i)
    {
        i32 bit = read_bit(img_data);
        if(bit == -1)
        {
            return 255;
        }
        code = (code << 1) | bit;
        for(u32 j = 0; j < ht->code_length_count[i]; ++j)
        {
            if(code == ht->huff_codes[idx_in_codes++])
            {
                return ht->huff_symbols[--idx_in_codes];
            }
        }
    }
    return 255;
}

// TODO: Need to think about error checking with this
static void process_mcu_component(jpg_img_data *img_data, i32 *color_comp, i32 *prev_dc_coeff,
                                  huff_table dc_table, huff_table ac_table)
{
    // Read huffman code and translate it to a symbol
    u8 length = get_next_symbol(img_data, &dc_table);
    i32 dc_coeff = read_bits(img_data, length);
    if(dc_coeff == -1)
    {
        OutputDebugStringA("Error reading bits for DC_COEFFICIENT\n");
        return;
    }
    if(length && dc_coeff < (1 << (length - 1)))
    {
        dc_coeff -= (1 << length) - 1;
    }
    color_comp[0] = dc_coeff + *prev_dc_coeff;
    *prev_dc_coeff = color_comp[0];

    // AC values for MCU
    for(u32 i = 1; i < 64; ++i)
    {
        u8 symbol = get_next_symbol(img_data, &ac_table);
        // Remaining symbols are zero
        if(symbol == 0x00)
        {
            for(; i < 64; ++i)
            {
                color_comp[zz_grouping[i]] = 0;
            }
            return; // Done once we fill with zeros
        }

        u8 num_of_zeros = get_upper_nibble(symbol);
        u8 ac_coeff_length = get_lower_nibble(symbol);
        i32 ac_coeff = 0;
        if(symbol == 0xF0)
        {
            num_of_zeros = 16;
        }
        // Zero run length could exceed mcu?

        for(u32 j = 0; j < num_of_zeros; ++j, ++i)
        {
            color_comp[zz_grouping[i]] = 0;
        }
        // AC coeff len cannot be >10

        if(ac_coeff_length)
        {
            ac_coeff = read_bits(img_data, ac_coeff_length);
            if(ac_coeff == -1)
            {
                OutputDebugStringA("Error reading ac_coeff bits\n");
                return;
            }
            if(ac_coeff < (1 << (ac_coeff_length - 1)))
            {
                ac_coeff -= (1 << ac_coeff_length) - 1;
            }
            color_comp[zz_grouping[i]] = ac_coeff;
        }
    }
}

static mcu *decode_huff_data(memory_arena *ma, jpg_info *info)
{
    // NOTE: For now I am going to assume all the jpgs are going to be divisible by 8
    if (info->image_width % 8 != 0 || info->image_height % 8 != 0)
    {
        OutputDebugStringA("WARN: jpg does not equally divide into 8x8 mcus... Will not decode properly.");
    }
    // NOTE: Gets the number of mcu image tiles
    u32 mcu_height = info->image_height / 8;
    u32 mcu_width = info->image_width / 8;
    mcu *result = push_array(ma, (mcu_height*mcu_width), mcu);

    // NOTE: Get huff codes from symbols
    for(u32 i = 0; i < 4; ++i)
    {
        gen_huff_codes(&info->dc_tables[i]);
        gen_huff_codes(&info->ac_tables[i]);
    }

    // Handles using prev DC coeff to get true DC coeff
    i32 prev_dc_coeff[3] = {};
    // NOTE: Extract coeff for each color component
    for(u32 i = 0; i < (mcu_height*mcu_width); ++i)
    {
        if(info->restart_inverval_between_mcus && i % info->restart_inverval_between_mcus == 0)
        {
            prev_dc_coeff[0] = 0;
            prev_dc_coeff[1] = 0;
            prev_dc_coeff[2] = 0;
            align_bytes(info->img_data);
        }
        process_mcu_component(info->img_data, result[i].y, &prev_dc_coeff[0], info->dc_tables[info->components[0].id], info->ac_tables[info->components[0].id]);
        process_mcu_component(info->img_data, result[i].cb, &prev_dc_coeff[1], info->dc_tables[info->components[1].id], info->ac_tables[info->components[1].id]);
        process_mcu_component(info->img_data, result[i].cr, &prev_dc_coeff[2], info->dc_tables[info->components[2].id], info->ac_tables[info->components[2].id]);
    }
    return result;
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

                        info.img_data = push_struct(ma, jpg_img_data);
                        u8 *ba_image_data = bytes;
                        u8 *ba_file = (u8 *)read_result.contents;
                        u32 image_data_size = (u32)(read_result.contents_size - (ba_image_data - ba_file));
                        info.img_data->data = push_array(ma, image_data_size, u8);
                        info.img_data->data_size = store_raw_huff_data(&bytes, info.img_data->data);
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
    // Decode Huffman Data
    mcu *mcus = decode_huff_data(ma, &info);

    return result;
}


