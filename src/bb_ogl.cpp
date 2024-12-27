#pragma once
#include "core.unity.h"

static GLuint create_ogl_shader_program(thread_context tc, char *vertex_file_name, char *fragment_file_name)
{
  GLuint prog_id = 0;
  debug_file_result s_vertex_file = DEBUG_read_entire_file(&tc, vertex_file_name);
  debug_file_result s_fragment_file = DEBUG_read_entire_file(&tc, fragment_file_name);

  // Create shaders
  const char *vertexShaderSource = (char *)s_vertex_file.contents;
  u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertex_shader);
  // Check the shader was compiled successfully
  s32 success;
  char info[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    char put_string[512];
    glGetShaderInfoLog(vertex_shader, ArrayCount(info), NULL, info);
    _snprintf_s(put_string, sizeof(put_string), "Failed vertex shader compilation: %s\n",
                info);
    OutputDebugStringA(put_string);
  }
  const char *fragmentShaderSource = (char *)s_fragment_file.contents;
  u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    char put_string[512];
    glGetShaderInfoLog(fragment_shader, ArrayCount(info), NULL, info);
    _snprintf_s(put_string, sizeof(put_string), "Failed fragment shader compilation: %s\n",
                info);
    OutputDebugStringA(put_string);
  }
  // Create shader program
  prog_id = glCreateProgram();
  glAttachShader(prog_id, vertex_shader);
  glAttachShader(prog_id, fragment_shader);
  glLinkProgram(prog_id);
  glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
  if(!success)
  {
    char put_string[512];
    glGetProgramInfoLog(prog_id, ArrayCount(info), NULL, info);
    _snprintf_s(put_string, sizeof(put_string), "Failed shader program link: %s\n",
                info);
    OutputDebugStringA(put_string);
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  DEBUG_free_file(&tc, s_vertex_file.contents);
  DEBUG_free_file(&tc, s_fragment_file.contents);

  return prog_id;
}

static void init_renderer(void) {
  return;
}

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

