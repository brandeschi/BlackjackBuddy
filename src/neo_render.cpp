#pragma once

#include "core.unity.h"

internal void InitRenderer(renderer *Renderer, thread_context *Thread, app_memory *Memory)
{
  Renderer->units = 0;
  Renderer->unit_count = 0;

  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");
}
