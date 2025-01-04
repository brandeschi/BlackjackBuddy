// Platform stuff

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
      u32 file_size32 = SafeTruncateU64(file_size.QuadPart);
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


// TODO: This is not final bmp loading code!
static loaded_bmp DEBUG_load_bmp(thread_context *thread, debug_read_entire_file *read_entire_file, char *file_name)
{
  loaded_bmp result = {};
  debug_file_result read_result = read_entire_file(thread, file_name);
  if (read_result.contents_size != 0)
  {
    bmp_header *header = (bmp_header *)read_result.contents;
    u8 *pixels = ((u8 *)read_result.contents + header->bitmap_offset);
    u32 *test_pix = (u32 *)pixels;


    result.pixels = pixels;
    result.channels = header->bits_per_pixel / 8;
    result.width = header->width;
    result.height = header->height;

    // NOTE: For some reason I did not have to do any shifting of the bits for my bmp file?
    // If I always take images and export them through gimp, I should be good.
#if 0
    // NOTE: Byte order of the bmp pixels in memory is deteremined by the header itself!
    u32 alpha_mask = ~(header->red_mask | header->green_mask | header->blue_mask);

    bit_scan_result red_shift = find_least_sig_set_bit_32(header->red_mask);
    bit_scan_result green_shift = find_least_sig_set_bit_32(header->green_mask);
    bit_scan_result blue_shift = find_least_sig_set_bit_32(header->blue_mask);
    bit_scan_result alpha_shift = find_least_sig_set_bit_32(alpha_mask);

    NeoAssert(red_shift.found);
    NeoAssert(green_shift.found);
    NeoAssert(blue_shift.found);
    NeoAssert(alpha_shift.found);

    u32 *src_dest = (u32 *)pixels;
    for (s32 Y = 0; Y < header->height; Y++)
    {
      for (s32 X = 0; X < header->width; X++)
      {
        u32 color = *src_dest;
        *src_dest++ = ((((color >> alpha_shift.index) & 0xFF) << 24) |
          (((color >> red_shift.index) & 0xFF) << 16) |
          (((color >> green_shift.index) & 0xFF) << 8) |
          (((color >> blue_shift.index) & 0xFF) << 0)
        );
      }
    }
#endif
  }


  return result;
}
