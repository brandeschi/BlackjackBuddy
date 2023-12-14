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

// NOTE: Debug file handling
DEBUG_FREE_FILE_MEMORY(DEBUG_free_file)
{
    if (file)
    {
        VirtualFree(file, 0, MEM_RELEASE);
    }
}

DEBUG_READ_ENTIRE_FILE(DEBUG_read_entire_file)
{
    debug_file_result result = {};
    HANDLE file_handle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size))
        {
           u32 file_size32 = safe_truncate_int64(file_size.QuadPart);
           result.contents = VirtualAlloc(0, file_size32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
           if (result.contents)
           {
               DWORD bytes_read;
               if (ReadFile(file_handle, result.contents, file_size32, &bytes_read, 0) &&
                   (file_size32 == bytes_read))
               {
                   result.contents_size = file_size32;
               }
               else
               {
                   DEBUG_free_file(thread, result.contents);
                   result.contents = 0;
               }
           }
        }
    }
    CloseHandle(file_handle);
    return result;
}

DEBUG_WRITE_ENTIRE_FILE(DEBUG_write_entire_file)
{
    b32 result = false;
    HANDLE file_handle = CreateFile(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        DWORD bytes_written;
        if (WriteFile(file_handle, file, file_size, &bytes_written, 0))

        {
            result = (bytes_written == file_size);
        }
        else
        {
            // TODO: LOGGING
        }

    }

    CloseHandle(file_handle);
    return result;
}
