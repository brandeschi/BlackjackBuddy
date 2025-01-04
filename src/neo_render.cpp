#pragma once

#include "core.unity.h"

// NOTE: Only expecting to have one renderer. This could change in the future.
static renderer Renderer = {0};

internal void InitRenderer(thread_context *Thread, app_memory *Memory)
{
  // NOTE: Each card is like 98 hori and 153 vert
  Renderer.tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");
}
