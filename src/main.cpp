#include <stdio.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

#include "types.h"
#include "memory.h"
#include "strings.h"
#include "file_io.h"

#define FILE_EXT ".bmp"
// TODO: put this in a Colour struct or something? so I can do Colour::White, etc.
#define COLOUR_WHITE v4f(1.0f, 1.0f, 1.0f)
#define COLOUR_BLACK v4f(0.0f, 0.0f, 0.0f)
#define COLOUR_RED  v4f(1.0f, 0.0f, 0.0f)
#define COLOUR_BLUE v4f(0.0f, 0.0f, 1.0f)
#define COLOUR_GREEN v4f(0.0f, 1.0f, 0.0f)
#define COLOUR_CYAN v4f(0.0f, 1.0f, 1.0f)

struct Ray
{
    Ray(v3f rayOrigin, v3f rayDir) : origin(rayOrigin), dir(rayDir) {}
    
    v3f at(f32 t) { return origin + dir*t; }
    
    v3f origin;
    v3f dir;
};

struct Sphere
{
    v3f pos;
    f32 radius;
};

static inline void setPixel(Image* image, u32 x, u32 y, v4f colour)
{
    image->pixels[y*image->width + x] = colour;
}

static inline void fillImage(Image* image, v4f colour)
{
    for (u32 i = 0; i < image->width*image->height; ++i)
    {
        *(image->pixels + i) = colour;
    }
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
    if (!stringEndsWith(fileName, FILE_EXT))
        fileName = concatStrings(fileName, FILE_EXT);
    else
        duplicateString(argv[1]);
    
    printf("File to save to: %s\n", fileName);
    
    Image image = {};
    image.width = 640;
    image.height = 400;
    image.pixels = (v4f*)allocMemory(sizeof(v4f)*image.width*image.height);
    
    fillImage(&image, COLOUR_WHITE);
    
    Sphere sphere = {};
    sphere.pos = v3f(0.5f, -0.1f, -3.0f);
    sphere.radius = 1.5f;
    
    // NOTE: we use a Y up coordinate system where +X is to the right and the camera points
    // into the negative Z direction
    v3f cameraPos = v3f();
    
    // distance between camera and image plane
    f32 focalLength = 1.0f;
    
    f32 imagePlaneHeight = 2.0f;
    f32 imagePlaneWidth = imagePlaneHeight*((f32)image.width/image.height);
    
    f32 pixelSize = imagePlaneWidth/image.width;
    assert(imagePlaneWidth/image.width == imagePlaneHeight/image.height);
    
    f32 startImagePlaneX = -imagePlaneWidth/2 + pixelSize/2; 
    f32 startImagePlaneY = imagePlaneHeight/2 - pixelSize/2;
    f32 imagePlaneX = startImagePlaneX;
    f32 imagePlaneY = startImagePlaneY;
    
    for (u32 imageY = 0; imageY < image.height; ++imageY)
    {
        for (u32 imageX = 0; imageX < image.width; ++imageX)
        {
            v3f imagePlanePoint = v3f(imagePlaneX, imagePlaneY, -focalLength);
            
            Ray ray = Ray(cameraPos, normalize(imagePlanePoint - cameraPos));
            
            f32 t = F32_MIN;
            
            // sphere intersection test
            f32 a = dot(ray.dir, ray.dir);
            f32 b = 2*dot(ray.dir, ray.origin - sphere.pos);
            f32 c = dot(ray.origin - sphere.pos, ray.origin - sphere.pos) - sphere.radius*sphere.radius;
            
            f32 discriminant = b*b - 4*a*c;
            if (discriminant > 0)
            {
                // two sphere collision points
                
                f32 rootValue = (f32)sqrt(discriminant);
                
                f32 t1 = (-b + rootValue) / (2*a);
                f32 t2 = (-b - rootValue) / (2*a);
                
                // NOTE: right now I don't render spheres that are behind or straddling the
                // camera. Not sure if the second case is something I should handle or not.
                if (t1 > 0 && t2 > 0)
                {
                    t = MIN_VALUE(t1, t2);
                    
                    // TODO: this is a duplicate of the below code, so I should probably
                    // refactor this out
                    v3f intersectPoint = ray.at(t);
                    v3f normal = intersectPoint - sphere.pos;
                    
                    // for now the colour will just be normal value
                    v4f colour = v4f(normalize(normal));
                    colour = 0.5f*(colour + v4f(1.0f, 1.0f, 1.0f, 1.0f));
                    setPixel(&image, imageX, imageY, colour);
                }
            }
            else if (discriminant == 0)
            {
                // one sphere collision point
                t = -b / (2*a);
                
                if (t > 0)
                {
                    v3f intersectPoint = ray.at(t);
                    v3f normal = intersectPoint - sphere.pos;
                    
                    // for now the colour will just be normal value
                    v4f colour = v4f(normalize(normal));
                    colour = 0.5f*(colour + v4f(1.0f, 1.0f, 1.0f, 1.0f));
                    setPixel(&image, imageX, imageY, colour);
                }
            }
            
            // if no collisions we draw a simple gradient
            if (t < 0)
            {
                f32 ratio = 0.5f*(ray.dir.y + 1.0f);
                v4f colour = (1.0f - ratio)*COLOUR_WHITE + ratio*COLOUR_CYAN;
                setPixel(&image, imageX, imageY, colour);
            }
            
            imagePlaneX += pixelSize;
        }
        
        imagePlaneX = startImagePlaneX;
        imagePlaneY -= pixelSize;
    }
    
    write_image_to_bmp(fileName, &image);
    
    freeMemory(fileName);
    freeMemory(image.pixels);
    
    return 0;
}