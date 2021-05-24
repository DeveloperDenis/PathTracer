#include "geometry.h"

Ray::Ray(v3f rayOrigin, v3f rayDir, bool normalized) : origin(rayOrigin), dir(rayDir)
{
    if (!normalized)
        rayDir = normalize(rayDir);
}

static f32 intersection_test(Ray* ray, Sphere* sphere)
{
    f32 tResult = F32_MIN;
    
    // TODO: could reduce number of calculations if I precalculate some values and simplify
    // the quadratic formula by doing some substitution and working it out
    f32 a = 1; //dot(ray.dir, ray.dir); not needed because ray.dir is normalized
    f32 b = 2*dot(ray->dir, ray->origin - sphere->pos);
    f32 c = dot(ray->origin - sphere->pos, ray->origin - sphere->pos) - sphere->radius*sphere->radius;
    
    f32 discriminant = b*b - 4*a*c;
    if (discriminant > 0)
    {
        // two sphere collision points
        
        f32 rootValue = (f32)sqrt(discriminant);
        
        // NOTE: We only really need the closest intersection point, and if it is negative,
        // then we don't want to draw the sphere anyway because it's either behind the camera
        // or envelopping the camera
        tResult = (-b - rootValue) / (2*a);
    }
    else if (discriminant == 0)
    {
        // one sphere collision point
        tResult = -b / (2*a);
    }
    
    return tResult;
}

static f32 intersection_test(Ray* ray, Plane* plane)
{
    f32 tResult = F32_MIN;
    
    f32 denominator = dot(ray->dir, plane->normal);
    if (denominator != 0.0f)
    {
        tResult = (plane->offset - dot(plane->normal, ray->origin)) / denominator;
    }
    
    return tResult;
}