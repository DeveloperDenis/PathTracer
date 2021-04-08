#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// copied from float.h file
#define F32_MAX 3.402823466e+38F
#define F32_MIN -3.402823466e+38F

#define MAX_VALUE(value1, value2) ((value1) < (value2) ? (value2) : (value1))
#define MIN_VALUE(value1, value2) ((value1) < (value2) ? (value1) : (value2))

#define ABS_VALUE(value) ((value) < 0 ? (-(value)) : (value))

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

typedef float f32;
typedef double f64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// TODO: these should maybe be in a "math" header instead
#include <math.h>
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
    
    v3f operator+(v3f v)
    {
        v3f result;
        result.x = x + v.x;
        result.y = y + v.y;
        result.z = z + v.z;
        return result;
    }
    
    v3f operator-(v3f v)
    {
        v3f result = v3f();
        result.x = x - v.x;
        result.y = y - v.y;
        result.z = z - v.z;
        return result;
    }
    
    v3f operator*(f32 value)
    {
        v3f result;
        result.x = x * value;
        result.y = y * value;
        result.z = z * value;
        return result;
    }
    
    v3f operator/(f32 value)
    {
        return *this * (1.0f/value);
    }
    
    v3f operator-()
    {
        v3f result = {};
        result.x = -x;
        result.y = -y;
        result.z = -z;
        return result;
    }
};

static inline f32 norm_squared(v3f v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}
static inline f32 norm(v3f v)
{
    return (f32)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline f32 distance_squared(v3f v1, v3f v2)
{
    return norm_squared(v2 - v1);
}
static inline f32 distance(v3f v1, v3f v2)
{
    return norm(v2 - v1);
}

static inline v3f normalize(v3f v)
{
    return v / norm(v);
}

static inline f32 dot(v3f v1, v3f v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

static inline bool near_zero(v3f v)
{
    const f32 error = 1e-8f;
    return ABS_VALUE(v.x) < error && ABS_VALUE(v.y) < error && ABS_VALUE(v.z) < error;
}

union v4f
{
    static v4f one() {
        return v4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
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
    
    v4f operator+(v4f v)
    {
        v4f result;
        result.x = x + v.x;
        result.y = y + v.y;
        result.z = z + v.z;
        result.w = w + v.w;
        return result;
    }
    
    v4f operator-(v4f v)
    {
        v4f result;
        result.x = x - v.x;
        result.y = y - v.y;
        result.z = z - v.z;
        result.w = w - v.w;
        return result;
    }
    
    v4f operator*(f32 value)
    {
        v4f result;
        result.x = x*value;
        result.y = y*value;
        result.z = z*value;
        result.w = w*value;
        return result;
    }
    
    v4f operator/(f32 value)
    {
        return *this * (1.0f/value);
    }
    
    v4f& operator+=(v4f right)
    {
        *this = *this + right;
        return *this;
    }
};

v4f operator*(f32 value, v4f v)
{
    return v*value;
}

static inline v4f hadamard(v4f v1, v4f v2)
{
    v4f result = {};
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    result.w = v1.w * v2.w;
    return result;
}

struct Image
{
    v4f* pixels;
    u32 width;
    u32 height;
};

#endif //TYPES_H
