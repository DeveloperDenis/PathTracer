#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

// forward declaration for return type
struct Ray;

struct Camera
{
    Camera();
    Camera(v3f pos, f32 fovDegrees, f32 aspectRatio, f32 focalLength = 1.0f);
    
    v3f pos;
    v3f dir; // normalized vector for the camera's facing direction
    v3f up; //  normalized vector pointing in the camera's up direction
    
    f32 focalLength; // distance between the camera and the image plane
    f32 fov; // the vertical field-of-view of the camera, in radians
    f32 aspectRatio; // the aspect ratio of the image plane, width/height
    
    v3f image_plane_pos();
    
    v2f image_plane_dim();
    f32 image_plane_width() { return image_plane_dim().w; }
    f32 image_plane_height() { return image_plane_dim().h; }
    
    void set_target(v3f targetPos);
    
    // get ray from camera pos intersecting through (u, v) coords on image plane with origin in top-left
    Ray get_ray(f32 u, f32 v);
};

#endif //CAMERA_H
