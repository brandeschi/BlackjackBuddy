struct win32_bitmap_buffer
{
    BITMAPINFO info; // BITMAPINFO bmi; this is a struct
    void *memory;
    int pitch;
    int bytes_per_pixel;
    int width;
    int height;
};

struct win32_win_dimensions
{
    int width;
    int height;
};

struct win32_sound_settings
{
    int samples_per_sec;
    u32 running_sample_index;
    int bytes_per_sample;
    DWORD secondary_buffer_size;
    DWORD cushion_bytes;
};
