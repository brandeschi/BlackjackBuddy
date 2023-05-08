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


