// NOTE: Jpg markers
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

u8 zz_grouping[] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

struct component_info
{
    u8 id;
    u8 qt_table_id;
    u8 huff_DCtable_id;
    u8 huff_ACtable_id;
    u8 huff_table_id;
    u8 h_sampling;
    u8 v_sampling;
};
struct quant_table
{
    // NOTE: If 0, the table uses bytes... If 1, the table uses words...
    u8 precision_index;
    u8 id;
    u32 table_values[64];
};
struct huff_table
{
    // NOTE: Read top nibble; if >0, then it is an AC table else it is a DC table
    u8 id;
    u16 num_of_symbols;
    u8 code_length_count[16];
    u8 huff_symbols[162];
};

struct jpg_info
{
    quant_table quant_tables[4];
    huff_table dc_tables[4];
    huff_table ac_tables[4];
    u32 *pixels;
    u8 *raw_img_data;
    u16 restart_inverval_between_mcus;
    b32 grayscale;
    u8 num_of_qt_tables;
    u8 bits_per_sample;
    u16 image_width;
    u16 image_height;
    component_info components[3];
};


