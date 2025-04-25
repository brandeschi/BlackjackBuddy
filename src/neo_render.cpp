#pragma once

#include "core.unity.h"

// TODO: Reduce this once I batch render units
const global u32 MAX_UNITS = KB(5);

// TODO: This makes me think I should catergorize each unit so that way at the end,
// I can stuff all the data for units of the same kind into their respective glBufferSubData
// calls and then execute the draw call for each type of unit.
internal void PushQuad(renderer *Renderer, vertex_data *Vertices, mat4 Model)
{
  memory_arena *FrameArena = &Renderer->frame_arena;
  u32 UnitCount = Renderer->unit_count;

  render_unit *Unit = PushStruct(FrameArena, render_unit);

  u32 EboIndexPattern[] =
  {
    0 + (4*UnitCount), 1 + (4*UnitCount), 3 + (4*UnitCount),
    1 + (4*UnitCount), 2 + (4*UnitCount), 3 + (4*UnitCount),
  };

  u32 EboIndexCount = 6;
  Unit->indices = PushArray(FrameArena, EboIndexCount, u32);
  Unit->index_count = EboIndexCount;
  memcpy(Unit->indices, EboIndexPattern, EboIndexCount*sizeof(u32));

  u32 VertexCount = 4;
  Unit->vertices = PushArray(FrameArena, VertexCount, vertex_data);
  Unit->vertex_count = VertexCount;

  // Apply Model Transform
  for (s32 Index = 0; Index < (s32)VertexCount; ++Index)
  {
    v4 Current = V4FromV3(Vertices[Index].position);
    Vertices[Index].position = V3FromV4(Model*Current);
  }

  memcpy(Unit->vertices, Vertices, VertexCount*sizeof(vertex_data));

  Unit->model = Model;
  // NOTE: Since I am using an arena and reusing memory for the exact same
  // struct type, I need to make sure all fields are zero'd out to ensure
  // no old state messes with the new pushes
  // TODO: Make ZeroStruct macro for structs in arenas?
  Unit->next = 0;

  if (UnitCount == 0)
  {
    Renderer->head = Unit;
    Renderer->tail = Unit;
  }
  else
  {
    render_unit *LastUnit = Renderer->tail;
    LastUnit->next = Unit;
    Renderer->tail = Unit;
  }
  ++Renderer->unit_count;
}

internal void PushCard(renderer *Renderer, v2 CardCoords = {0}, mat4 Model = Mat4Iden())
{
  f32 CardWidth = Renderer->card_width;
  f32 CardHeight = Renderer->card_height;

  vertex_data Vertices[] =
  {
    // pos                                             color               tex-coords
    { {0.0f - (CardWidth), 0.0f - (CardHeight), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
    { {0.0f + (CardWidth), 0.0f - (CardHeight), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
    { {0.0f + (CardWidth), 0.0f + (CardHeight), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
    { {0.0f - (CardWidth), 0.0f + (CardHeight), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
  };

  CardWidth /= 0.21f;
  CardHeight /= 0.21f;
  loaded_bmp CardAtlas = Renderer->card_atlas;
  v2 Min = { (CardCoords.x * CardWidth) / CardAtlas.width, (CardCoords.y * CardHeight) / CardAtlas.height };
  v2 Max = { ((CardCoords.x + 1) * CardWidth) / CardAtlas.width, ((CardCoords.y + 1) * CardHeight) / CardAtlas.height };
  Vertices[0].tex_coords = { Min.x, Min.y };
  Vertices[1].tex_coords = { Max.x, Min.y };
  Vertices[2].tex_coords = { Max.x, Max.y };
  Vertices[3].tex_coords = { Min.x, Max.y };

  PushQuad(Renderer, Vertices, Model);
}

internal void PushText(renderer *Renderer, string Text, mat4 Model = Mat4Scale(0.5f, 0.5f, 1.0f))
{
  f32 ScreenX = -1.0f, ScreenY = 60.0f;
  for (s32 Index = 0; Index < Text.count; ++Index)
  {
    s32 CurrentCharIdx = Text.data[Index] - 33;
    stbtt_aligned_quad Quad;
    stbtt_GetBakedQuad(Renderer->chars, 512, 512, CurrentCharIdx, &ScreenX, &ScreenY, &Quad, 1);
    stbtt_bakedchar CurrentBakedChar = Renderer->chars[CurrentCharIdx];

    f32 GlyphHeight = Quad.y1 - Quad.y0;
    f32 YAdjust = CurrentBakedChar.yoff + (GlyphHeight);
    f32 GlyphY0 = Quad.y0 - YAdjust;
    f32 GlyphY1 = Quad.y1 - YAdjust;

    // vertex_data VData[] =
    // {
    //   // pos                      color               tex-coords
    //   { {Quad.x0, GlyphY0, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s0, Quad.t1}, 1.0f }, // Top-Left
    //   { {Quad.x1, GlyphY0, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s1, Quad.t1}, 1.0f }, // Top-Right
    //   { {Quad.x1, GlyphY1, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s1, Quad.t0}, 1.0f }, // Bottom-Right
    //   { {Quad.x0, GlyphY1, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s0, Quad.t0}, 1.0f }, // Bottom-Left
    // };
    vertex_data VData[] =
    {
      // pos                      color               tex-coords
      { {Quad.x0, Quad.y0, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s0, Quad.t1}, 1.0f }, // Top-Left
      { {Quad.x1, Quad.y0, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s1, Quad.t1}, 1.0f }, // Top-Right
      { {Quad.x1, Quad.y1, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s1, Quad.t0}, 1.0f }, // Bottom-Right
      { {Quad.x0, Quad.y1, 0.0f}, {1.0f, 1.0f, 1.0f}, {Quad.s0, Quad.t0}, 1.0f }, // Bottom-Left
    };
    // // if (Text.data[Index] == 'a')
    // // {
    //   VData[0].position.y += (-1.0f)*CurrentBakedChar.yoff + (Quad.y1 - Quad.y0);
    //   VData[1].position.y += (-1.0f)*CurrentBakedChar.yoff + (Quad.y1 - Quad.y0);
    //   VData[2].position.y += (-1.0f)*CurrentBakedChar.yoff + (Quad.y1 - Quad.y0);
    //   VData[3].position.y += (-1.0f)*CurrentBakedChar.yoff + (Quad.y1 - Quad.y0);
    // // }

    PushQuad(Renderer, VData, Model);
  }
}

internal void InitRenderer(thread_context *Thread, app_memory *Memory, renderer *Renderer)
{
  InitArena(&Renderer->frame_arena, MB(4), Memory->flex_memory);

  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->card_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");
  Renderer->width = 960;
  Renderer->height = 540;
  Renderer->card_width = (f32)Renderer->card_atlas.width / 13.0f * 0.21f;
  Renderer->card_height = (f32)Renderer->card_atlas.height / 5.0f * 0.21f;
  Renderer->max_units = MAX_UNITS;

#if 0
  // Create all bitmap structs for character glyphs

  stbtt_fontinfo FontInfo;
  stbtt_InitFont(&FontInfo, (u8 *)TTFontFile.contents, stbtt_GetFontOffsetForIndex((u8 *)TTFontFile.contents, 0));

  // Make BMP for all letters
  for (char Current = 'A'; Current <= 'Z'; ++Current)
  {
    loaded_bmp GlyphBitmap = {0};
    GlyphBitmap.channels = 4;
    s32 Width, Height;
    u8 *StbBitmap = stbtt_GetCodepointBitmap(&FontInfo, 0, stbtt_ScaleForPixelHeight(&FontInfo, 64.0f),
                                             Current, &Width, &Height, 0, 0);

    GlyphBitmap.width = (u32)Width;
    GlyphBitmap.height = (u32)Height;
    // TODO: Better memory place for this.
    GlyphBitmap.pixels = (u8 *)malloc(4*GlyphBitmap.width*GlyphBitmap.height);

    u8 *Src = StbBitmap;
    u8 *EndOfBmp = GlyphBitmap.pixels + (4*GlyphBitmap.width*(GlyphBitmap.height - 1));
    for (ums Row = 0; Row < GlyphBitmap.height; ++Row)
    {
      u32 *Dest = (u32 *)EndOfBmp;
      for (ums Col = 0; Col < GlyphBitmap.width; ++Col)
      {
        u8 MonoPixel = *Src++;
        *Dest++ = (
          (MonoPixel << 24) |
          (MonoPixel << 16) |
          (MonoPixel <<  8) |
          (MonoPixel <<  0));
      }

      EndOfBmp -= 4*GlyphBitmap.width;
    }


    Renderer->font_glyphs[Current - 65] = GlyphBitmap;
    stbtt_FreeBitmap(StbBitmap, 0);
  }
#endif
}

internal void ResetRenderer(renderer *Renderer)
{
  if (Renderer->unit_count == 0) return;
  memory_arena *Arena = &Renderer->frame_arena;
  InitArena(Arena, Arena->size, Arena->base);
  Renderer->head = 0;
  Renderer->unit_count = 0;
}
