#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"

struct Ray
{
    Ray(v3f rayOrigin, v3f rayDir, bool normalized = true) ;
    
    v3f at(f32 t) { return origin + dir*t; }
    
    v3f origin;
    v3f dir;
};

struct Sphere
{
    Sphere() : pos(v3f()), radius(1.0f) {}
    Sphere(v3f p, f32 r) : pos(p), radius(r) {}
    
    v3f pos;
    f32 radius;
};

struct Plane
{
    v3f normal;
    f32 offset;
};

/**
*   FUNCTIONS
*/

// reflect a direction vector about a normal
static inline v3f reflect_direction(v3f dir, v3f normal)
{
    f32 projectedDistance = -dot(dir, normal);
    v3f reflectedDir = dir + 2.0f*normal*projectedDistance;
    return reflectedDir;
}

// return a reflected ray about the given normal
static inline Ray reflect_ray(Ray* ray, v3f point, v3f normal)
{
    return Ray(point, reflect_direction(ray->dir, normal), false);
}


static f32 intersection_test(Ray* ray, Sphere* sphere);
static f32 intersection_test(Ray* ray, Plane* plane);

#endif //GEOMETRY_H
