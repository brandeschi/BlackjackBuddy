#pragma once

#include "core.unity.h"

static void DrawCard(vertex_data *VertexArray, loaded_bmp TexAtlas, v2 CardIndex) {
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;

  v2 ComputedTexCoords[] = {
    {(CardIndex.x * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height },
    {(CardIndex.x * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height }
  };
  for (u32 Index = 0; Index < 4; ++Index) {
    VertexArray[Index].tex_coords = ComputedTexCoords[Index];
  }
}

// TODO: Probably want to move the creation of this to the platform layer
// when we are picking which rendering API to use.
// NOTE: Only expecting to have one renderer. This could change in the future.

internal void InitRenderer(thread_context *Thread, app_memory *Memory, renderer *Renderer)
{
  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");

  // Create Shader Program
  g_ShaderProgram = CreateOpenGLShaderProgram(Thread, "..\\vert.glsl", "..\\frag.glsl");
  // Vertex Data
  // v3 vertices[] = {
  //     { -0.5f, -0.5f, 0.0f }, // V1 pos data
  //     { 1.0f, 0.0f, 0.0f }, // V1 color data
  //     { 0.5f, -0.5f, 0.0f }, // V2 pos data
  //     { 0.0f, 1.0f, 0.0f }, // V2 color data
  //     { 0.0f, 0.5f, 0.0f }, // V3 pos data
  //     { 0.0f, 0.0f, 1.0f }, // V3 color data
  // };
  //
  // v2 tex_coords[] = {
  //     { 0.0f, 0.0f },
  //     { 1.0f, 0.0f },
  //     { 0.5f, 1.0f },
  // };
  //
  // v3 vertices[] = {
  //     { 0.5f, 0.5f, 0.0f },   // top-right
  //     { 0.5f, -0.5f, 0.0f },  // bottom-right
  //     { -0.5f, -0.5f, 0.0f }, // bottom-left
  //     { -0.5f, 0.5f, 0.0f },  // top-left
  // };

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

  u32 CurrentUnitSize = Renderer->unit_count*sizeof(render_unit);
  if (CurrentUnitSize + sizeof(render_unit) < Renderer->max_units)
  {
    render_unit *Unit = (render_unit *)Renderer->units;
    u32 *IndicesBase = (u32 *)(Renderer->units + sizeof(render_unit));
    u32 IndexCount = ArrayCount(Indices);
    // TODO: Remove memcpy?
    memcpy(IndicesBase, Indices, IndexCount*sizeof(u32));
    Unit->indices = IndicesBase;
    Unit->index_count = IndexCount;
    vertex_data *VerticesBase = (vertex_data *)((u8 *)IndicesBase + IndexCount*sizeof(u32));
    u32 VertexCount = ArrayCount(Vertices);
    memcpy(VerticesBase, Vertices, VertexCount*sizeof(vertex_data));
    Unit->vertices = VerticesBase;
    Unit->vertex_count = ArrayCount(Vertices);
    Unit->offset = Renderer->unit_count++;
  }
  else
  {
    InvalidCodePath
  }

  // Create a (V)ertex (B)uffer (O)bject and (V)ertex (A)rray (O)bject

  glGenVertexArrays(1, &Renderer->VAO);
  glGenBuffers(1, &Renderer->VBO);
  glGenBuffers(1, &Renderer->EBO);

  render_unit *FirstUnit = (render_unit *)Renderer->units;
  vertex_data *Vertices_ = FirstUnit->vertices;
  DrawCard(Vertices_, Renderer->tex_atlas, {0.0f, 4.0f});
  DrawCard(&Vertices_[4], Renderer->tex_atlas, {0.0f, 3.0f});

  // Bind VAO, the bind and set VBOs, then config vertex attribs
  glBindVertexArray(Renderer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
  glBufferData(GL_ARRAY_BUFFER, FirstUnit->vertex_count*sizeof(vertex_data), FirstUnit->vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, FirstUnit->index_count*sizeof(u32), FirstUnit->indices, GL_STATIC_DRAW);

  // Tell opengl how to interpret our vertex data by setting pointers to the attribs
  // pos attrib
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
  glEnableVertexAttribArray(0);
  // color attrib
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(3*sizeof(f32)));
  glEnableVertexAttribArray(1);
  // tex coord attrib
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(6*sizeof(f32)));
  glEnableVertexAttribArray(2);

  GLuint Texture;
  glGenTextures(1, &Texture);
  glBindTexture(GL_TEXTURE_2D, Texture);
  // glBindTexture(GL_TEXTURE_2D, 1);
  // Setting Texture wrapping method
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // Setting Texture filtering methods
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // TODO: GL_BGRA_EXT is a windows specific value, I believe I need to somehow handle this in the platform layer
  // or when I pull this rendering code out
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Renderer->tex_atlas.width, Renderer->tex_atlas.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Renderer->tex_atlas.pixels);

  const u32 Width = 960;
  const u32 Height = 540;
  mat4 Projection = Mat4Ortho(0.0f, (f32)Width, 0.0f, (f32)Height, -1.0f, 1.0f);
  mat4 MVP = Projection*Mat4Iden()*Mat4Iden();
  Renderer->width = Width;
  Renderer->height = Height;

  glUseProgram(g_ShaderProgram);
  GLint u_MvpId = glGetUniformLocation(g_ShaderProgram, "u_MVP");
  glUniformMatrix4fv(u_MvpId, 1, GL_FALSE, (f32 *)MVP.e);
  glUseProgram(0);
}
