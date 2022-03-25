#ifndef VECTORS_H
#define VECTORS_H

union v2f
{
    v2f() : x(0), y(0) {}
    v2f(f32 xVal, f32 yVal) : x(xVal), y(yVal) {}
    
    struct
    {
        f32 x;
        f32 y;
    };
    struct
    {
        f32 w;
        f32 h;
    };
    f32 e[2];
};

union v3f
{
    v3f() : x(0), y(0), z(0) {}
    v3f(f32 xVal, f32 yVal, f32 zVal) : x(xVal), y(yVal), z(zVal) {}
    
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 e[3];
};

union v4f
{
    v4f() : x(0), y(0), z(0), w(0) {}
    v4f(f32 xVal, f32 yVal, f32 zVal) : x(xVal), y(yVal), z(zVal), w(1.0) {}
    v4f(f32 xVal, f32 yVal, f32 zVal, f32 wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}
    explicit v4f(v3f v) : x(v.x), y(v.y), z(v.z), w(1.0) {}
    
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 e[4];
    
    v4f& operator+=(v4f right);
};

// overloaded operators

v3f operator+(v3f a, v3f b);
v3f operator-(v3f a, v3f b);
v3f operator*(v3f v, f32 value);
v3f operator*(f32 value, v3f v);
v3f operator/(v3f v, f32 value);
v3f operator-(v3f v);

v4f operator+(v4f a, v4f b);
v4f operator-(v4f a, v4f b);
v4f operator*(v4f v, f32 value);
v4f operator*(f32 value, v4f v);
v4f operator/(v4f v, f32 value);

// vector functions

f32 norm(v3f v);
f32 norm_squared(v3f v);

f32 distance(v3f v1, v3f v2);
f32 distance_squared(v3f v1, v3f v2);

v3f normalize(v3f v);

f32 dot(v3f v1, v3f v2);
v3f cross(v3f v1, v3f v2);

bool near_zero(v3f v);

v4f hadamard(v4f v1, v4f v2);

#endif //VECTORS_H
