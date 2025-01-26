// Math Constants
#define PI32 3.14159265359f

// Math types
union v2
{
  struct
  {
    f32 x, y;
  };
  struct
  {
    f32 u, v;
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

//NOTE: Matrices
// (Math style 4x4 - right handed, row major)
union mat4
{
  struct {
    f32 m0,  m1,  m2,  m3;
    f32 m4,  m5,  m6,  m7;
    f32 m8,  m9,  m10, m11;
    f32 m12, m13, m14, m15;
  };

  f32 e[4][4];
};

// TODO: Change to use intrinsic bit_scan
// Remove this after
struct bit_scan_result
{
  b32 found;
  u32 index;
};
