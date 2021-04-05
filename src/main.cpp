#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

#include "types.h"
#include "memory.h"
#include "strings.h"
#include "file_io.h"

#define FILE_EXT ".bmp"

// the higher this value is, the less aliasing in the final image
// but it will drastically increase the render time
#define SAMPLES_PER_PIXEL 32

#define MAX_RAY_DEPTH 10


// TODO: put this in a Colour struct or something? so I can do Colour::White, etc.
#define COLOUR_WHITE v4f(1.0f, 1.0f, 1.0f)
#define COLOUR_BLACK v4f(0.0f, 0.0f, 0.0f)
#define COLOUR_RED  v4f(1.0f, 0.0f, 0.0f)
#define COLOUR_BLUE v4f(0.0f, 0.0f, 1.0f)
#define COLOUR_GREEN v4f(0.0f, 1.0f, 0.0f)
#define COLOUR_CYAN v4f(0.0f, 1.0f, 1.0f)

static inline bool is_equal(f32 a, f32 b, f32 error = 0.0001f)
{
    return ABS_VALUE(a - b) <= error;
}

struct Ray
{
    Ray(v3f rayOrigin, v3f rayDir, bool normalized = true) 
        : origin(rayOrigin), dir(rayDir)
    {
        if (!normalized)
            rayDir = normalize(rayDir);
    }
    
    v3f at(f32 t) { return origin + dir*t; }
    
    v3f origin;
    v3f dir;
};

struct Sphere
{
    v3f pos;
    f32 radius;
};

f32 intersection_test(Ray* ray, Sphere* sphere)
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

// returns a random value in the range [0, 1)
static inline f32 random_f32()
{
    return (f32)rand() / (RAND_MAX + 1.0f);
}
// returns a random value in the range [min, max)
static inline f32 random_f32(f32 min, f32 max)
{
    assert(min < max);
    return random_f32() * (max - min) + min;
}

static inline v3f random_v3f()
{
    return v3f(random_f32(), random_f32(), random_f32());
}
static inline v3f random_v3f(f32 min, f32 max)
{
    return v3f(random_f32(min, max), random_f32(min, max), random_f32(min, max));
}

static v3f random_point_in_sphere(Sphere* sphere)
{
    v3f randomPoint = v3f();
    bool found = false;
    
    while (!found)
    {
        randomPoint = random_v3f(-sphere->radius, sphere->radius);
        
        if (norm(randomPoint) <= sphere->radius)
            found = true;
    }
    
    return randomPoint + sphere->pos;
}

static inline void set_pixel(Image* image, u32 x, u32 y, v4f colour)
{
    image->pixels[y*image->width + x] = colour;
}

static inline void fill_image(Image* image, v4f colour)
{
    for (u32 i = 0; i < image->width*image->height; ++i)
    {
        *(image->pixels + i) = colour;
    }
}

// returns colour of pixel after ray cast
static v4f cast_ray(Ray* ray, Sphere* objectList, u32 numObjects, u32 maxDepth = 1)
{
    v4f resultColour = v4f();
    
    if (maxDepth <= 0)
        return COLOUR_BLACK;
    
    const f32 MIN_T = 0.001f;
    const f32 MAX_T = F32_MAX;
    
    f32 tClosest = F32_MAX;
    v3f intersectNormal = v3f();
    v3f intersectPoint = v3f();
    
    // checking for intersection
    for (u32 i = 0; i < numObjects; ++i)
    {
        Sphere* sphere = objectList + i;
        f32 t = intersection_test(ray, sphere);
        
        if (t > MIN_T && t < tClosest)
        {
            tClosest = t;
            
            intersectPoint = ray->at(t);
            intersectNormal = normalize(intersectPoint - sphere->pos);
        }
    }
    
    // calculating colour for pixel
    if (tClosest != F32_MAX && tClosest > 0)
    {
        Sphere diffuseSphere = {};
        diffuseSphere.radius = 1.0f;
        diffuseSphere.pos = intersectPoint + intersectNormal;
        
        v3f newRayTargetPos = random_point_in_sphere(&diffuseSphere);
        
        Ray reflectRay = Ray(intersectPoint, newRayTargetPos - intersectPoint, false);
        // the 0.5 is the amount of light absorbed by the object on each bounce, 50% here
        resultColour = 0.5f*cast_ray(&reflectRay, objectList, numObjects, maxDepth - 1);
    }
    else
    {
        // if no collisions we draw a simple gradient
        f32 ratio = 0.5f*(ray->dir.y + 1.0f);
        resultColour = (1.0f - ratio)*COLOUR_WHITE + ratio*COLOUR_CYAN;
    }
    
    return resultColour;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("ERROR: No output file name given.\n");
        printf("USAGE: %s file_name\n", argv[0]);
        return 1;
    }
    
    char* fileName = argv[1];
    if (!string_ends_with(fileName, FILE_EXT))
        fileName = concat_strings(fileName, FILE_EXT);
    else
        duplicate_string(argv[1]);
    
    printf("File to save to: %s\n", fileName);
    
    Image image = {};
    image.width = 640;
    image.height = 400;
    image.pixels = (v4f*)memory_alloc(sizeof(v4f)*image.width*image.height);
    
    fill_image(&image, COLOUR_BLACK);
    
    Sphere sphereList[3];
    {
        Sphere sphere = {};
        sphere.pos = v3f(0.5f, -0.1f, -3.0f);
        sphere.radius = 1.5f;
        sphereList[0] = sphere;
    }
    {
        Sphere sphere = {};
        sphere.pos = v3f(-1.0f, 0.4f, -4.0f);
        sphere.radius = 1.5f;
        sphereList[1] = sphere;
    }
    {
        Sphere sphere = {};
        sphere.pos = v3f(0.0f, -102, -5.0f);
        sphere.radius = 100.0f;
        sphereList[2] = sphere;
    }
    
    // NOTE: we use a Y up coordinate system where +X is to the right and the camera points
    // into the negative Z direction
    v3f cameraPos = v3f();
    
    // distance between camera and image plane
    f32 focalLength = 1.0f;
    
    f32 imagePlaneHeight = 2.0f;
    f32 imagePlaneWidth = imagePlaneHeight*((f32)image.width/image.height);
    
    f32 pixelSize = imagePlaneWidth/image.width;
    assert(imagePlaneWidth/image.width == imagePlaneHeight/image.height);
    
    // top left of image plane
    f32 startImagePlaneX = -imagePlaneWidth/2; 
    f32 startImagePlaneY = imagePlaneHeight/2;
    
    f32 imagePlaneX = startImagePlaneX;
    f32 imagePlaneY = startImagePlaneY;
    
    for (u32 pixelY = 0; pixelY < image.height; ++pixelY)
    {
        for (u32 pixelX = 0; pixelX < image.width; ++pixelX)
        {
            v4f pixelColour = v4f();
            
            for (u32 sampleIndex = 0; sampleIndex < SAMPLES_PER_PIXEL; ++sampleIndex)
            {
                f32 u = random_f32()*pixelSize;
                f32 v = random_f32()*pixelSize;
                
                v3f imagePlanePoint = v3f(imagePlaneX + u, imagePlaneY - v, -focalLength);
                
                Ray ray = Ray(cameraPos, normalize(imagePlanePoint - cameraPos));
                
                pixelColour += cast_ray(&ray, sphereList, ARRAY_LENGTH(sphereList), MAX_RAY_DEPTH);
            }
            
            set_pixel(&image, pixelX, pixelY, pixelColour/SAMPLES_PER_PIXEL);
            
            imagePlaneX += pixelSize;
        }
        
        imagePlaneX = startImagePlaneX;
        imagePlaneY -= pixelSize;
    }
    
    write_image_to_bmp(fileName, &image);
    
    memory_free(fileName);
    memory_free(image.pixels);
    
    return 0;
}