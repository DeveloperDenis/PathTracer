#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define MATH_PI 3.141592653589793f

// copied from float.h file
#define F32_MAX 3.402823466e+38F
#define F32_MIN -3.402823466e+38F

#define MAX_VALUE(value1, value2) ((value1) < (value2) ? (value2) : (value1))
#define MIN_VALUE(value1, value2) ((value1) < (value2) ? (value1) : (value2))

#define ABS_VALUE(value) ((value) < 0 ? (-(value)) : (value))

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define DEGREES_TO_RADIANS(degrees) ((degrees) * (MATH_PI/180.0f))

#define SWAP(a, b, Type) do \
{ \
Type c = (a); \
(a) = (b); \
(b) = c; \
} while(0)

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

#endif //TYPES_H
