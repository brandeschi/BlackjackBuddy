#pragma once

#include "core.unity.h"

internal void SetCardTexCoord(loaded_bmp TexAtlas, vertex_data *Vertices, v2 CardIndex)
{
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;

  v2 ComputedTexCoords[] =
  {
    {(CardIndex.x * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height },
    {(CardIndex.x * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height }
  };
  for (u32 Index = 0; Index < 4; ++Index)
  {
    Vertices[Index].tex_coords = ComputedTexCoords[Index];
  }
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

  loaded_bmp TexAtlas = Renderer->tex_atlas;
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;
  v2 ScreenCenter = { (f32)Width / 2.0f, (f32)Height / 2.0f };
  vertex_data Vertices[] =
  {
    // pos                                                                                          color               tex-coords
    { {ScreenCenter.x - (CardWidth*0.25f), ScreenCenter.y - (CardHeight*0.25f), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
    { {ScreenCenter.x + (CardWidth*0.25f), ScreenCenter.y - (CardHeight*0.25f), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
    { {ScreenCenter.x + (CardWidth*0.25f), ScreenCenter.y + (CardHeight*0.25f), 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
    { {ScreenCenter.x - (CardWidth*0.25f), ScreenCenter.y + (CardHeight*0.25f), 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
  };
  u32 Indices[] =
  {
    0, 1, 3,    // T1
    1, 2, 3,    // T2
  };

  SetCardTexCoord(Renderer->tex_atlas, Vertices, { 0.0f, 2.0f });

  // TODO: Make some kind of PushUnit once it becomes apparent how I want
  // to push units to the Renderer.
  memory_arena *FrameArena = &Renderer->frame_arena;
  // TODO: Renderer now uses a linked list however, I want to test
  // at some point if using the linked list is better or worse than
  // having all the units next to each other at the front of the arena.
  Renderer->max_units = 128;
  render_unit *FirstUnit = PushStruct(FrameArena, render_unit);
  Renderer->head = FirstUnit;
  u32 IndexCount = ArrayCount(Indices);
  FirstUnit->indices = PushArray(FrameArena, IndexCount, u32);
  FirstUnit->index_count = IndexCount;
  memcpy(FirstUnit->indices, Indices, IndexCount*sizeof(u32));
  u32 VertexCount = ArrayCount(Vertices);
  FirstUnit->vertices = PushArray(FrameArena, VertexCount, vertex_data);
  FirstUnit->vertex_count = VertexCount;
  memcpy(FirstUnit->vertices, Vertices, VertexCount*sizeof(vertex_data));
  ++Renderer->unit_count;
}
