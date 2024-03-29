// Basic Math Lib
// NOTE: Even though we don't use this I still want to know what exactly this did!
// f32 &operator[](int index) { return ((&x)[index]); }
// inline v2 &operator*=(f32 real);
// inline v2 &operator+=(v2 a);

union v2
{
  struct
  {
    f32 x, y;
  };
  f32 e[2];
};

union v3
{
  struct
  {
    f32 x, y, z;
  };
  struct
  {
    f32 r, g, b;
  };
  f32 e[3];
};

union v4
{
  struct
  {
    f32 x, y, z, w;
  };
  struct
  {
    f32 r, g, b, a;
  };
  f32 e[4];
};

// NOTE: A C++ 11 way of doing the below would be
// v2 A = v2{1, 2};
inline v2 V2(f32 x, f32 y)
{
  v2 result;
  result.x = x;
  result.y = y;
  return result;
}

inline v3 V3(f32 x, f32 y, f32 z)
{
  v3 result;
  result.x = x;
  result.y = y;
  result.z = z;
  return result;
}

inline v4 V4(f32 x, f32 y, f32 z, f32 w)
{
  v4 result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
  return result;
}

inline v2 operator*(f32 real, v2 a)
{
  v2 result;
  result.x = real * a.x;
  result.y = real * a.y;
  return result;
}

inline v2 operator*(v2 a, f32 real)
{
  v2 result = real * a;
  return result;
}

inline v2 &operator*=(v2 &a, f32 real)
{
  a = real * a;
  return a;
}

inline v2 operator-(v2 a)
{
  v2 result;
  result.x = -a.x;
  result.y = -a.y;
  return result;
}

inline v2 operator+(v2 a, v2 b)
{
  v2 result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

inline v2 &operator+=(v2 &a, v2 b)
{
  a = a + b;
  return a;
}

inline v2 operator-(v2 a, v2 b)
{
  v2 result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

inline f32 squared(f32 input)
{
  f32 result = input * input;
  return result;
}

inline f32 dot_product(v2 a, v2 b)
{
  f32 result = a.x*b.x + a.y*b.y;
  return result;
}

inline f32 length_sq(v2 a)
{
  f32 result = dot_product(a, a);
  return result;
}

//NOTE: Matrices
// (OpenGL style 4x4 - right handed, column major)
union mat4
{
  struct {
    f32 m0,  m1,  m2,  m3;
    f32 m4,  m5,  m6,  m7;
    f32 m8,  m9,  m10, m11;
    f32 m12, m13, m14, m15;
  };

  f32 e[16];
};

inline mat4 mat4_iden()
{
  mat4 result = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return result;
}

inline mat4 mat4_multiply(mat4 left, mat4 right)
{
  mat4 result = {};

  result.m0 =  left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
  result.m1 =  left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
  result.m2 =  left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
  result.m3 =  left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
  result.m4 =  left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
  result.m5 =  left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
  result.m6 =  left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
  result.m7 =  left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
  result.m8 =  left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
  result.m9 =  left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
  result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
  result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
  result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
  result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
  result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
  result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

  return result;
}

inline mat4 operator*(mat4 left, mat4 right)
{
  mat4 result = {};
  result = mat4_multiply(left, right);

  return result;
}

inline mat4 mat4_translate(f32 x, f32 y, f32 z)
{
  mat4 result = {
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return result;
}

inline mat4 mat4_scale(f32 x, f32 y, f32 z)
{
  mat4 result = {
    x, 0.0f, 0.0f, 0.0f,
    0.0f, y, 0.0f, 0.0f,
    0.0f, 0.0f, z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return result;
}

// TODO:
// inline mat4 mat4_rotate();

inline mat4 mat4_ortho(f32 left, f32 right, f32 top, f32 bottom, f32 nearP, f32 farP)
{
  f32 rl_range = right - left;
  f32 tb_range = top - bottom;
  f32 fn_range = farP - nearP;

  // NOTE: This is colunm-major(OGL inforced); which means the rows in these map to columns in math notation.
  mat4 result = {
    (2.0f/rl_range),              0.0f,                         0.0f,                       0.0f,
    0.0f,                         (2.0f/tb_range),              0.0f,                       0.0f,
    0.0f,                         0.0f,                         (-2.0f/fn_range),           0.0f,
    -((right + left)/(rl_range)), -((top + bottom)/(tb_range)), -((farP + nearP)/(fn_range)), 1.0f
  };

  return result;
}
