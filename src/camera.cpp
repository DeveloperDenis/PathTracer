#include "camera.h"
#include "geometry.h"
#include "utils.h"

Camera::Camera()
{
    pos = v3f();
    dir = v3f(0.0f, 0.0f, -1.0f); // camera pointing in the -Z direction
    up = v3f(0.0f, 1.0f, 0.0f); // Y up camera, perfectly level
    
    fov = MATH_PI/2;
    aspectRatio = 1.0f; // default to a square aspect ratio
    
    focusDistance = 1.0f; // no focus distance effect
    lensRadius = 0.0f; // pin-hole camera, no blur
}

Camera::Camera(v3f pos, f32 fovDegrees, f32 aspectRatio)
{
    this->pos = pos;
    this->dir = v3f(0.0f, 0.0f, -1.0f);
    this->up = v3f(0.0f, 1.0f, 0.0f);
    
    this->fov = DEGREES_TO_RADIANS(fovDegrees);
    this->aspectRatio = aspectRatio;
    
    focusDistance = 1.0f;
    lensRadius = 0.0f;
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
    
    return pos + dir*focusDistance;
}

void Camera::set_target(v3f targetPos)
{
    dir = normalize(targetPos - pos);
}

void Camera::set_lens(f32 apertureSize, f32 focusPlaneDist)
{
    this->lensRadius = apertureSize/2;
    this->focusDistance = focusPlaneDist;
}

Ray Camera::get_ray(f32 u, f32 v)
{
    v3f planePos = image_plane_pos();
    v2f planeDim = image_plane_dim();
    
    // NOTE: not sure if the normalize step here is needed, cross products of unit vectors give a unit vector
    // only if the two vectors are orthogonal
    v3f horizontal = normalize(cross(dir, up));
    v3f vertical = up;
    
    // in addition to moving the image plane along dir by the focus distance, we also have to scale the image plane
    // so that we have a consistent frame size no matter the focus distance
    v3f topLeft = planePos - horizontal*(planeDim.w/2.0f)*focusDistance + vertical*(planeDim.h/2.0f)*focusDistance;
    v3f rayTarget = topLeft + u*horizontal*planeDim.w*focusDistance - v*vertical*planeDim.h*focusDistance;
    
    // starting ray from a random point on the lens, if the aperture is set
    assert(lensRadius >= 0.0f);
    v3f pointOnLens = random_point_in_unit_circle()*lensRadius;
    v3f lensOffset = pointOnLens.x*horizontal + pointOnLens.y*vertical;
    
    Ray result = Ray(pos + lensOffset, normalize(rayTarget - (pos + lensOffset)));
    return result;
}