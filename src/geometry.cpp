#include "geometry.h"

Ray::Ray(v3f rayOrigin, v3f rayDir, bool normalized) : origin(rayOrigin), dir(rayDir)
{
    if (!normalized)
        this->dir = normalize(rayDir);
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

static f32 intersection_test(Ray* ray, Rect3f* rect)
{
    f32 tResult = F32_MIN;
    
    // TODO: What should I do in the case where the ray origin is on a rectangle edge?
    
    f32 inverseDirX = 1.0f/ray->dir.x;
    f32 tx0 = (rect->left() - ray->origin.x)*inverseDirX;
    f32 tx1 = (rect->right() - ray->origin.x)*inverseDirX;
    
    if (tx0 > tx1)
        SWAP(tx0, tx1, f32);
    
    f32 inverseDirY = 1.0f/ray->dir.y;
    f32 ty0 = (rect->bottom() - ray->origin.y)*inverseDirY;
    f32 ty1 = (rect->top() - ray->origin.y)*inverseDirY;
    
    if (ty0 > ty1)
        SWAP(ty0, ty1, f32);
    
    f32 inverseDirZ = 1.0f/ray->dir.z;
    f32 tz0 = (rect->back() - ray->origin.z)*inverseDirZ;
    f32 tz1 = (rect->front() - ray->origin.z)*inverseDirZ;
    
    if (tz0 > tz1)
        SWAP(tz0, tz1, f32);
    
    f32 tMin = MAX_VALUE(MAX_VALUE(tx0, ty0), tz0);
    f32 tMax = MIN_VALUE(MIN_VALUE(tx1, ty1), tz1);
    
    if (tMin < tMax)
        tResult = tMin;
    
    return tResult;
}