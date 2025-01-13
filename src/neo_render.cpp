#pragma once

#include "core.unity.h"

// TODO: Probably want to move the creation of this to the platform layer
// when we are picking which rendering API to use.
// NOTE: Only expecting to have one renderer. This could change in the future.

internal void InitRenderer(thread_context *Thread, app_memory *Memory, renderer *Renderer)
{
  // NOTE: Each card is like 98 hori and 153 vert
  Renderer->tex_atlas = DEBUG_load_bmp(Thread, Memory->DEBUG_read_entire_file, "test/cards.bmp");

  // Create Shader Program
  g_ShaderProgram = CreateOpenGLShaderProgram(Thread, "..\\vert.glsl", "..\\frag.glsl");
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

  // TODO: Look into nailing this down.
  // Also I think I should use an arena for managing the unit memory.
  u32 CurrentUnitSize = Renderer->unit_count*sizeof(render_unit);
  if (CurrentUnitSize + sizeof(render_unit) < Renderer->max_units)
  {
    render_unit *Unit = (render_unit *)Renderer->units;
    u32 *IndicesBase = (u32 *)(Renderer->units + sizeof(render_unit));
    u32 IndexCount = ArrayCount(Indices);
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

  render_unit *FirstUnit = (render_unit *)(Renderer->units);

  // Bind VAO, the bind and set VBOs, then config vertex attribs
  glBindVertexArray(Renderer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
  glBufferData(GL_ARRAY_BUFFER, FirstUnit->vertex_count*sizeof(vertex_data), FirstUnit->vertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, FirstUnit->index_count*sizeof(u32), FirstUnit->indices, GL_DYNAMIC_DRAW);

  // Tell opengl how to interpret our vertex data by setting pointers to the attribs
  // pos attrib
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);

  // color attrib
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(3*sizeof(f32)));
  // tex coord attrib
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(6*sizeof(f32)));

  u32 TextureAtlas;
  glGenTextures(1, &TextureAtlas);
  glBindTexture(GL_TEXTURE_2D, TextureAtlas);

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

  // Unbind the buffers
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}
