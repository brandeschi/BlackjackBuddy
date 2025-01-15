#pragma once

#include "core.unity.h"

internal void InitRenderer(thread_context *Thread, app_memory *Memory, renderer *Renderer)
{
  InitArena(&Renderer->frame_arena, megabytes(4), PlatformAllocateMemory(megabytes(4)));

  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");

  u32 Width = 960;
  u32 Height = 540;
  Renderer->width = Width;
  Renderer->height = Height;

  vertex_data Vertices[] = {
    //  pos                     color               tex-coords
    { {100.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
    { {150.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
    { {150.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
    { {100.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left

    { {200.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
    { {250.0f, 125.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
    { {250.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
    { {200.0f, 50.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
  };
  u32 Indices[] = {
    0, 1, 3,    // T1
    1, 2, 3,    // T2

    4, 5, 7,    // T3
    5, 6, 7     // T4
  };

  // TODO: Make some kind of PushUnit once it becomes apparent how I want
  // to push units to the Renderer.
  memory_arena *FrameArena = &Renderer->frame_arena;
  render_unit *FirstUnit = PushStruct(FrameArena, render_unit);
  u32 IndexCount = ArrayCount(Indices);
  FirstUnit->indices = PushArray(FrameArena, IndexCount, u32);
  FirstUnit->index_count = IndexCount;
  memcpy(FirstUnit->indices, Indices, IndexCount*sizeof(u32));
  u32 VertexCount = ArrayCount(Vertices);
  FirstUnit->vertices = PushArray(FrameArena, VertexCount, vertex_data);
  FirstUnit->vertex_count = VertexCount;
  memcpy(FirstUnit->vertices, Vertices, VertexCount*sizeof(vertex_data));
  // TODO: Find some way to be able to move through the arena to get to each unit
  FirstUnit->offset = Renderer->unit_count++;
  Renderer->units = FirstUnit;
}
