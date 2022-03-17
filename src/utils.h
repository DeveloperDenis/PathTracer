#ifndef UTILS_H
#define UTILS_H

#include <random>

#include "types.h"

static inline bool is_equal(f32 a, f32 b, f32 error = 0.0001f)
{
    return ABS_VALUE(a - b) <= error;
}

// returns a random value in the range [0, 1)
static inline f64 random_f64()
{
    static thread_local std::mt19937_64 generator;
    
    std::uniform_real_distribution<f64> distribution(0.0, 1.0);
    
    return distribution(generator);
}

// returns a random value in the range [0, 1)
static inline f32 random_f32()
{
    static thread_local std::mt19937 generator;
    
    std::uniform_real_distribution<f32> distribution(0.0f, 1.0f);
    
    return distribution(generator);
}
// returns a random value in the range [min, max)
static inline f32 random_f32(f32 min, f32 max)
{
    assert(min < max);
    return random_f32() * (max - min) + min;
}

// returns a random value in the range [0, scale)
static inline u32 random_u32(u32 scale)
{
    return (u32)((rand() / (RAND_MAX + 1.0)) * scale);
}

static inline u32 random_u32(u32 min, u32 max)
{
    assert(min < max);
    return (u32)random_f32((f32)min, (f32)max);
}

static inline s32 random_s32(s32 min, s32 max)
{
    assert(min < max);
    return (s32)random_f32((f32)min, (f32)max);
}

static inline v3f random_v3f()
{
    return v3f(random_f32(), random_f32(), random_f32());
}
static inline v3f random_v3f(f32 min, f32 max)
{
    return v3f(random_f32(min, max), random_f32(min, max), random_f32(min, max));
}

static v3f random_point_in_unit_sphere()
{
    v3f randomPoint = v3f();
    bool found = false;
    
    while (!found)
    {
        randomPoint = random_v3f(-1.0f, 1.0f);
        
        if (norm(randomPoint) <= 1.0f)
            found = true;
    }
    
    return randomPoint;
}

static v3f random_point_in_sphere(Sphere* sphere)
{
    v3f randomPoint = random_point_in_unit_sphere();
    return randomPoint*sphere->radius + sphere->pos;
}

static v3f random_unit_vector()
{
    // TODO: couldn't I just use random_v3f and then normalize that instead?
    v3f randomPoint = random_point_in_unit_sphere();
    randomPoint = normalize(randomPoint);
    return randomPoint;
}

static v3f random_point_in_hemisphere(Sphere* sphere, v3f hemisphereNormal)
{
    v3f randomPoint = random_point_in_unit_sphere()*sphere->radius;
    
    // if the hemisphere is on the wrong side of the normal, we reflect the point about 
    // the centre
    if (dot(randomPoint, hemisphereNormal) < 0.0f)
    {
        randomPoint = -randomPoint;
    }
    
    return randomPoint + sphere->pos;
}

static v3f random_point_in_unit_circle()
{
    v3f randomPoint = v3f();
    bool found = false;
    
    while (!found)
    {
        randomPoint = v3f(random_f32(-1, 1), random_f32(-1, 1), 0);
        
        if (norm(randomPoint) <= 1.0f)
            found = true;
    }
    
    return randomPoint;
}

#endif //UTILS_H
