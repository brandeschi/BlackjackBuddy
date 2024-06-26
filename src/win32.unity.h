#pragma once

#include <math.h>
#include <stdint.h>

#include <Windows.h>
// NOTE: Need to undef b/c they are old macros win windows.h.
#include <stdio.h>
#include <dsound.h>
#include <gl/GL.h>
#include <stdlib.h>
#include <time.h>

#include "neo_platform.h"

#include "neo_math.h"
#include "math_utils.h"
#include "nebula_random.h"

#include "win32_nebula.h"
#include "nebula.h"

global b32 g_running = false;
global win32_bitmap_buffer g_bm_buffer;
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
global thread_context g_thread_context = {};
global GLuint g_shader_program;
global i64 g_perf_count_freq;

#include "bb_ogl.cpp"
#include "nebula.cpp"

