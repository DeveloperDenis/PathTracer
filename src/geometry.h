#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"

struct Ray
{
    Ray(v3f rayOrigin, v3f rayDir, bool normalized = true);
    
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

struct Rect3f
{
    // create a Rect3f from a minimum and maximum corner
    static Rect3f from_bounds(v3f min, v3f max);
    
    Rect3f() : pos(v3f()), halfWidth(0.5f), halfHeight(0.5f), halfLength(0.5f)  {}
    Rect3f(v3f pos, f32 width, f32 height, f32 length) : pos(pos), halfWidth(width*0.5f), halfHeight(height*0.5f), halfLength(length*0.5f) {}
    
    f32 width() { return halfWidth*2.0f; }
    f32 height() { return halfHeight*2.0f; }
    f32 length() { return halfLength*2.0f; }
    
    f32 left() { return pos.x - halfWidth; }
    f32 right() { return pos.x + halfWidth; }
    f32 bottom() { return pos.y - halfHeight; }
    f32 top() { return pos.y + halfHeight; }
    f32 back() { return pos.z - halfLength; }
    f32 front() { return pos.z + halfLength; }
    
    v3f pos;
    f32 halfWidth, halfHeight, halfLength;
};

/**
*   FUNCTIONS
*/

Rect3f bounding_box(Rect3f rect1, Rect3f rect2);
Rect3f bounding_box(Sphere sphere, f32 startTime = 0.0f, f32 endTime = 0.0f);

// reflect a direction vector about a normal
static inline v3f reflect_direction(v3f dir, v3f normal)
{
    f32 projectedDistance = -dot(dir, normal);
    v3f reflectedDir = dir + 2.0f*normal*projectedDistance;
    return reflectedDir;
}

// return a reflected ray about the given normal
static inline Ray reflect_ray(Ray ray, v3f point, v3f normal)
{
    return Ray(point, reflect_direction(ray.dir, normal), false);
}


static f32 intersection_test(Ray ray, Sphere sphere);
static f32 intersection_test(Ray ray, Plane plane);
static f32 intersection_test(Ray ray, Rect3f rect);

#endif //GEOMETRY_H
