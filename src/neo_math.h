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

