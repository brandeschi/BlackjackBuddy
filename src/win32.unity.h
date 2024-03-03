#pragma once

#include <math.h>
#include <stdint.h>

#define global static
#define pi32 3.14159265359f
#define KEEB_COUNT 1

#include <Windows.h>
// NOTE: Need to undef b/c they are old macros win windows.h.
#include <stdio.h>
#include <dsound.h>
#include <gl/GL.h>
#include <stdlib.h>
#include <time.h>

#include "nebula.h"
#include "win32_nebula.h"

global b32 g_running = false;
global win32_bitmap_buffer g_bm_buffer;
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
global thread_context g_thread_context = {};
global GLuint g_shader_program;
global i64 g_perf_count_freq;

#include "bb_ogl.cpp"
#include "nebula.cpp"

