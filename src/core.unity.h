#pragma once

// libC + vendor headers
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <gl/GL.h>
#include <dsound.h>

// Type Definitions!
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32;

typedef size_t mem_index;
typedef mem_index ums;

typedef float f32;
typedef double f64;

// Helper Macros
#define global static
#define internal static

#define PI32 3.14159265359f

#define NeoAssert(expression) if (!(expression)) { *(int *)0 = 0; }
#define InvalidCodePath NeoAssert(!"InvalidCodePath!");

#define ArrayCount(array) (sizeof((array)) / sizeof((array)[0]))
#define Swap(T,a,b) do{T temp__ = a; a = b; b = temp__;}while(0)


// Headers
#include "neo_math.h"
#include "neo_random.h"
#include "bb_ogl.h"
#include "bjbuddy.h"


// Sub TU
#include "neo_math.cpp"
#include "bb_ogl.cpp"
#include "bjbuddy.cpp"

