#pragma once

#include "core.unity.h"

const global u32 MAX_UNITS = 128;

internal void SetCardTexCoords(loaded_bmp TexAtlas, vertex_data *Vertices, v2 CardIndex)
{
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;

  v2 Min = { (CardIndex.x * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height };
  v2 Max = { ((CardIndex.x + 1) * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height };
  Vertices[0].tex_coords =  { Min.x, Min.y };
  Vertices[1].tex_coords = { Max.x, Min.y };
  Vertices[2].tex_coords = { Max.x, Max.y };
  Vertices[3].tex_coords = { Min.x, Max.y };
}

// TODO: This makes me think I should catergorize each unit so that way at the end,
// I can stuff all the data for units of the same kind into their respective glBufferSubData
// calls and then execute the draw call for each type of unit.
internal void PushQuad(renderer *Renderer, v2 TexCoords, mat4 Model = Mat4Iden())
{
  u32 Width = Renderer->width;
  u32 Height = Renderer->height;

  loaded_bmp TexAtlas = Renderer->tex_atlas;
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;
  v2 ScreenCenter = { (f32)Width / 2.0f, (f32)Height / 2.0f };

  vertex_data Vertices[] =
    {
      // pos                                                                             color               tex-coords
      { {ScreenCenter.x - (CardWidth*0.20f), ScreenCenter.y - (CardHeight*0.20f), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
      { {ScreenCenter.x + (CardWidth*0.20f), ScreenCenter.y - (CardHeight*0.20f), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
      { {ScreenCenter.x + (CardWidth*0.20f), ScreenCenter.y + (CardHeight*0.20f), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
      { {ScreenCenter.x - (CardWidth*0.20f), ScreenCenter.y + (CardHeight*0.20f), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
    };

  u32 EboIndexPattern[] =
  {
    0, 1, 3,
    1, 2, 3,
  };

  SetCardTexCoords(TexAtlas, Vertices, TexCoords);

  // TODO: Make some kind of PushUnit once it becomes apparent how I want
  // to push units to the Renderer.
  memory_arena *FrameArena = &Renderer->frame_arena;
  // TODO: Renderer now uses a linked list however, I want to test
  // at some point if using the linked list is better or worse than
  // having all the units next to each other at the front of the arena.
  render_unit *Unit = PushStruct(FrameArena, render_unit);

  u32 EboIndexCount = ArrayCount(EboIndexPattern);
  u32 *Indices = PushArray(FrameArena, EboIndexCount, u32);
  Unit->indices = Indices;
  Unit->index_count = EboIndexCount;
  u32 UnitCount = Renderer->unit_count;
  // TODO: Use this path eventually!
#if 0
  for (ums Index = 0; Index < EboIndexCount; ++Index)
  {
    Indices[Index] = EboIndexPattern[Index] + (4*UnitCount);
  }
#else
  memcpy(Unit->indices, EboIndexPattern, EboIndexCount*sizeof(u32));
#endif

  u32 VertexCount = ArrayCount(Vertices);
  Unit->vertices = PushArray(FrameArena, VertexCount, vertex_data);
  Unit->vertex_count = VertexCount;
  memcpy(Unit->vertices, Vertices, VertexCount*sizeof(vertex_data));
  Unit->model = Model;

  if (UnitCount == 0)
  {
    Renderer->head = Unit;
  }
  else
  {
    render_unit *LastUnit = Renderer->head;
    while (LastUnit->next != 0)
    {
      LastUnit = LastUnit->next;
    }
    LastUnit->next = Unit;
  }
  ++Renderer->unit_count;
}

internal void InitRenderer(thread_context *Thread, app_memory *Memory, renderer *Renderer)
{
  InitArena(&Renderer->frame_arena, megabytes(4), PlatformAllocateMemory(megabytes(4)));

  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");

  u32 Width = 960;
  u32 Height = 540;
  Renderer->width = Width;
  Renderer->height = Height;

  Renderer->max_units = MAX_UNITS;
}

internal void ResetRenderer(renderer *Renderer)
{
  if (Renderer->unit_count == 0) return;
  memory_arena Arena = Renderer->frame_arena;
  InitArena(&Arena, Arena.size, Arena.base);
  Renderer->head = 0;
  Renderer->unit_count = 0;
}
