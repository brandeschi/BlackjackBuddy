#pragma once

#include "win32.unity.h"
#include "bb_ogl.h"

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
    i32 success;
    char info[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char put_string[512];
        glGetShaderInfoLog(vertex_shader, arr_count(info), NULL, info);
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
        glGetShaderInfoLog(fragment_shader, arr_count(info), NULL, info);
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
        glGetProgramInfoLog(prog_id, arr_count(info), NULL, info);
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

static void draw_card(vertex_data *vertex_array, loaded_bmp tex_atlas, v2 card_index) {
    u32 quad_vertex_count = 4;
    f32 card_width = (f32)tex_atlas.width / 13.0f;
    f32 card_height = (f32)tex_atlas.height / 5.0f;

    v2 computed_tex_coords[] = {
        {(card_index.x * card_width) / tex_atlas.width, (card_index.y * card_height) / tex_atlas.height },
        {((card_index.x + 1) * card_width) / tex_atlas.width, (card_index.y * card_height) / tex_atlas.height },
        {((card_index.x + 1) * card_width) / tex_atlas.width, ((card_index.y + 1) * card_height) / tex_atlas.height },
        {(card_index.x * card_width) / tex_atlas.width, ((card_index.y + 1) * card_height) / tex_atlas.height }
    };
    for (u32 i = 0; i < quad_vertex_count; ++i) {
        vertex_array[i].tex_coords = computed_tex_coords[i];
    }
}

