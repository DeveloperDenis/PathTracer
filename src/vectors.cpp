#include "vectors.h"

// v3f operators

v3f operator+(v3f a, v3f b)
{
    v3f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

v3f operator-(v3f a, v3f b)
{
    v3f result = v3f();
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

v3f operator*(v3f v, f32 value)
{
    v3f result;
    result.x = v.x * value;
    result.y = v.y * value;
    result.z = v.z * value;
    return result;
}
v3f operator*(f32 value, v3f v)
{
    return v*value;
}

v3f operator/(v3f v, f32 value)
{
    return v * (1.0f/value);
}

v3f operator-(v3f v)
{
    v3f result = {};
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

// v4f operators

v4f& v4f::operator+=(v4f right)
{
    *this = *this + right;
    return *this;
}

v4f operator+(v4f a, v4f b)
{
    v4f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

v4f operator-(v4f a, v4f b)
{
    v4f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

v4f operator*(v4f v, f32 value)
{
    v4f result;
    result.x = v.x*value;
    result.y = v.y*value;
    result.z = v.z*value;
    result.w = v.w*value;
    return result;
}
v4f operator*(f32 value, v4f v)
{
    return v*value;
}

v4f operator/(v4f v, f32 value)
{
    return v * (1.0f/value);
}

// regular functions

f32 norm(v3f v)
{
    return (f32)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

f32 norm_squared(v3f v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}

f32 distance(v3f v1, v3f v2)
{
    return norm(v2 - v1);
}

f32 distance_squared(v3f v1, v3f v2)
{
    return norm_squared(v2 - v1);
}

v3f normalize(v3f v)
{
    return v / norm(v);
}

f32 dot(v3f v1, v3f v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

v3f cross(v3f v1, v3f v2)
{
    return v3f(v1.y*v2.z - v1.z*v2.y, 
               v1.z*v2.x - v1.x*v2.z, 
               v1.x*v2.y - v1.y*v2.x);
}

bool near_zero(v3f v)
{
    const f32 error = 1e-8f;
    return ABS_VALUE(v.x) < error && ABS_VALUE(v.y) < error && ABS_VALUE(v.z) < error;
}

v4f hadamard(v4f v1, v4f v2)
{
    v4f result = {};
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    result.w = v1.w * v2.w;
    return result;
}