#include "camera.h"
#include "geometry.h"

Camera::Camera()
{
    pos = v3f();
    dir = v3f(0.0f, 0.0f, -1.0f); // camera pointing in the -Z direction
    up = v3f(0.0f, 1.0f, 0.0f); // Y up camera, perfectly level
    
    focalLength = 1.0f;
    fov = MATH_PI/2;
    aspectRatio = 1.0f; // default to a square aspect ratio
}

Camera::Camera(v3f pos, f32 fovDegrees, f32 aspectRatio, f32 focalLength)
{
    this->pos = pos;
    this->dir = v3f(0.0f, 0.0f, -1.0f);
    this->up = v3f(0.0f, 1.0f, 0.0f);
    
    this->fov = degrees_to_radians(fovDegrees);
    this->focalLength = focalLength;
    this->aspectRatio = aspectRatio;
}

v2f Camera::image_plane_dim()
{
    // TODO: save this in a private variable or something so that it isn't constantly re-calculated?
    f32 height = 2.0f*(f32)(tan(fov/2.0f));
    f32 width = height*aspectRatio;
    
    return v2f(width, height);
}

v3f Camera::image_plane_pos()
{
    return pos + dir*focalLength;
}

void Camera::set_target(v3f targetPos)
{
    dir = normalize(targetPos - pos);
}

Ray Camera::get_ray(f32 u, f32 v)
{
    v3f planePos = image_plane_pos();
    v2f planeDim = image_plane_dim();
    
    // NOTE: not sure if the normalize step here is needed, cross products of unit vectors give a unit vector
    // only if the two vectors are orthogonal
    v3f horizontal = normalize(cross(dir, up));
    v3f vertical = up;
    
    v3f topLeft = planePos - horizontal*(planeDim.w/2.0f) + vertical*(planeDim.h/2.0f);
    v3f rayTarget = topLeft + u*horizontal - v*vertical;
    
    Ray result = Ray(pos, normalize(rayTarget - pos));
    return result;
}