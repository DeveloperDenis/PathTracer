#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

// forward declaration for return type
struct Ray;

struct Camera
{
    Camera();
    Camera(v3f pos, f32 fovDegrees, f32 aspectRatio);
    
    v3f pos;
    v3f dir; // normalized vector for the camera's facing direction
    v3f up; //  normalized vector pointing in the camera's up direction
    
    f32 fov; // the vertical field-of-view of the camera, in radians
    f32 aspectRatio; // the aspect ratio of the image plane, width/height
    
    f32 focusDistance; // the distance from the lens to the focus plane
    
    v3f image_plane_pos();
    
    v2f image_plane_dim();
    f32 image_plane_width() { return image_plane_dim().w; }
    f32 image_plane_height() { return image_plane_dim().h; }
    
    void set_target(v3f targetPos);
    void set_lens(f32 aperture, f32 focusDistance);
    
    // get ray from camera pos intersecting through (u, v) coords on image plane with origin in top-left
    Ray get_ray(f32 u, f32 v);
    
    private:
    
    f32 lensRadius;
};

#endif //CAMERA_H
