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
#include "geometry.cpp"
#include "camera.cpp"

#define FILE_EXT ".bmp"

#define RAY_TRACER_QUALITY 0

#if RAY_TRACER_QUALITY == 1
#define SAMPLES_PER_PIXEL 128
#define MAX_RAY_DEPTH 15
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720
#else
#define SAMPLES_PER_PIXEL 8
#define MAX_RAY_DEPTH 5
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 360
#endif

// TODO: put this in a Colour struct or something? so I can do Colour::White, etc.
#define COLOUR_WHITE v4f(1.0f, 1.0f, 1.0f)
#define COLOUR_BLACK v4f(0.0f, 0.0f, 0.0f)
#define COLOUR_RED  v4f(1.0f, 0.0f, 0.0f)
#define COLOUR_BLUE v4f(0.0f, 0.0f, 1.0f)
#define COLOUR_GREEN v4f(0.0f, 1.0f, 0.0f)
#define COLOUR_CYAN v4f(0.0f, 1.0f, 1.0f)
#define COLOUR_GOLD v4f(0.94f, 0.76f, 0.11f)

static inline bool is_equal(f32 a, f32 b, f32 error = 0.0001f)
{
    return ABS_VALUE(a - b) <= error;
}

struct Material
{
    // TODO: I don't know if I want to keep this as an enum, versus having something more
    // like the Principled shader in Blender
    enum Type
    {
        NONE,
        DIFFUSE,
        METAL,
        DIALECTRIC
    };
    
    Type type;
    v4f colour;
    
    // used by metal materials to control the fuzziness of reflections
    f32 roughness;
    
    // the refractive index for dialectric materials
    f32 n;
};

// TODO: Do i want each object to have it's own material, or do I want to have a master
// material list that each object has an index into?
struct RenderObject
{
    enum Type
    {
        SPHERE,
        PLANE
    };
    
    Material material;
    Type type;
    
    union
    {
        Sphere sphere;
        Plane plane;
    };
    
    RenderObject()
    {
        material = {};
        type = SPHERE;
        sphere = {};
    }
};

// TODO: perhaps also keep track of a material list?
struct World
{
    RenderObject list[128];
    u32 count;
    
    RenderObject* add_sphere(v3f pos, f32 radius, Material* material);
    RenderObject* add_plane(v3f normal, f32 d, Material* material);
};

RenderObject* World::add_sphere(v3f pos, f32 radius, Material* material)
{
    assert(material);
    assert(count < ARRAY_LENGTH(list));
    
    if (!material || count >= ARRAY_LENGTH(list))
        return 0;
    
    RenderObject* object = list + count;
    *object = {};
    
    object->type = RenderObject::Type::SPHERE;
    object->sphere.pos = pos;
    object->sphere.radius = radius;
    object->material = *material;
    
    ++count;
    
    return object;
}

RenderObject* World::add_plane(v3f normal, f32 d, Material* material)
{
    assert(material);
    assert(count < ARRAY_LENGTH(list));
    
    if (!material || count >= ARRAY_LENGTH(list))
        return 0;
    
    RenderObject* object = list + count;
    *object = {};
    object->type = RenderObject::Type::PLANE;
    object->plane.normal = normal;
    object->plane.offset = d;
    object->material = *material;
    
    ++count;
    
    return object;
}

static Material create_diffuse_material(v4f colour)
{
    Material result = {};
    result.type = Material::Type::DIFFUSE;
    result.colour = colour;
    return result;
}

static Material create_metal_material(v4f colour, f32 roughness = 0.0f)
{
    Material result = {};
    result.type = Material::Type::METAL;
    result.colour = colour;
    result.roughness = roughness;
    return result;
}

static Material create_dialectric_material(f32 refractiveIndex)
{
    Material result = {};
    result.type = Material::Type::DIALECTRIC;
    result.colour = COLOUR_WHITE; // TODO: do dialectrics always have no attenuation?
    result.n = refractiveIndex;
    return result;
}

// TODO: all these random functions could be put in a utility type file, or math file or something
// returns a random value in the range [0, 1)
static inline f64 random_f64()
{
    return rand() / (RAND_MAX + 1.0);
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

static v3f random_point_in_unit_sphere()
{
    v3f randomPoint = v3f();
    bool found = false;
    
    while (!found)
    {
        randomPoint = random_v3f(-1.0f, 1.0f);
        
        if (norm(randomPoint) <= 1.0f)
            found = true;
    }
    
    return randomPoint;
}

static v3f random_unit_vector()
{
    v3f randomPoint = random_point_in_unit_sphere();
    randomPoint = normalize(randomPoint);
    return randomPoint;
}

static v3f random_point_in_sphere(Sphere* sphere)
{
    v3f randomPoint = random_point_in_unit_sphere();
    return randomPoint*sphere->radius + sphere->pos;
}

static v3f random_point_in_hemisphere(Sphere* sphere, v3f hemisphereNormal)
{
    v3f randomPoint = random_point_in_unit_sphere()*sphere->radius;
    
    // if the hemisphere is on the wrong side of the normal, we reflect the point about 
    // the centre
    if (dot(randomPoint, hemisphereNormal) < 0.0f)
    {
        randomPoint = -randomPoint;
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


// calculates reflectance for a material using Schlick's Approximation
static f64 reflectance(f64 cosine, f64 refractRatio)
{
    f64 r0 = (1.0 - refractRatio) / (1.0 + refractRatio);
    r0 = r0*r0;
    
    return r0 + (1.0 - r0)*pow((1.0 - cosine), 5);
}

// returns colour of pixel after ray cast
static v4f cast_ray(Ray* ray, World* world, u32 maxDepth = 1)
{
    v4f resultColour = v4f();
    
    if (maxDepth <= 0)
        return COLOUR_BLACK;
    
    const f32 MIN_T = 0.001f;
    const f32 MAX_T = F32_MAX;
    
    f32 tClosest = F32_MAX;
    v3f intersectNormal = v3f();
    v3f intersectPoint = v3f();
    Material* material = 0;
    
    // checking for intersection
    for (u32 i = 0; i < world->count; ++i)
    {
        RenderObject* object = world->list + i;
        
        f32 t = F32_MIN;
        
        switch(object->type)
        {
            case RenderObject::Type::SPHERE:
            {
                Sphere* sphere = &object->sphere;
                t = intersection_test(ray, sphere);
            } break;
            
            case RenderObject::Type::PLANE:
            {
                Plane* plane = &object->plane;
                t = intersection_test(ray, plane);
            } break;
        }
        
        if (t > MIN_T && t < tClosest)
        {
            tClosest = t;
            
            intersectPoint = ray->at(t);
            
            if (object->type == RenderObject::Type::SPHERE)
                intersectNormal = normalize(intersectPoint - object->sphere.pos);
            else if (object->type == RenderObject::Type::PLANE)
                intersectNormal = object->plane.normal;
            
            material = &object->material;
        }
    }
    
    // calculating colour for pixel
    if (tClosest != F32_MAX && tClosest > 0)
    {
        assert(material);
        
        if (material->type == Material::Type::DIFFUSE)
        {
            v3f scatterDirection = random_unit_vector() + intersectNormal;
            if (near_zero(scatterDirection + intersectNormal))
                scatterDirection = intersectNormal;
            
            Ray reflectRay = Ray(intersectPoint, scatterDirection, false);
            v4f rayColour = cast_ray(&reflectRay, world, maxDepth - 1);
            
            // attenuate using the colour of the material
            resultColour = hadamard(material->colour, rayColour);
        }
        else if (material->type == Material::Type::METAL)
        {
            // the reflected ray is calculated assuming the surface is a perfect mirror
            v3f reflectedDir = reflect_direction(ray->dir, intersectNormal);
            
            if (material->roughness > 0.0f)
            {
                // we find a random point near the reflection point to make the reflection
                // less clear
                Sphere sphere = Sphere(intersectPoint + reflectedDir, material->roughness);
                v3f randomPoint = random_point_in_sphere(&sphere);
                
                reflectedDir = randomPoint - intersectPoint;
            }
            
            Ray reflectedRay = Ray(intersectPoint, reflectedDir, false);
            
            if (dot(reflectedDir, intersectNormal) > 0)
            {
                v4f rayColour = cast_ray(&reflectedRay, world, maxDepth - 1);
                resultColour = hadamard(material->colour, rayColour);
            }
            else
                resultColour = COLOUR_BLACK;
        }
        else if (material->type == Material::Type::DIALECTRIC)
        {
            // TODO: make this a formal parameter somewhere
            f32 worldIndex = 1.0f; // index of refraction of the world, air = 1.0
            
            f32 refractRatio = worldIndex/material->n;
            if (dot(ray->dir, intersectNormal) > 0.0f) // if ray and normal in same direction
                refractRatio = 1.0f/refractRatio;
            
            f32 cosTheta = dot(-ray->dir, intersectNormal);
            f32 sinTheta = (f32)sqrt(1 - cosTheta*cosTheta);
            
            v3f newRayDir = v3f();
            
            bool internalReflection  = refractRatio * sinTheta > 1.0f;
            // using Schlick's Approximation
            bool shouldReflect = reflectance(cosTheta, refractRatio) > random_f64();
            
            if (internalReflection || shouldReflect)
            {
                // Refraction impossible, so the ray must reflect
                newRayDir = reflect_direction(ray->dir, intersectNormal);
            }
            else
            {
                // Refraction!
                
                v3f rayPerpendicular = (refractRatio)*(ray->dir + cosTheta*intersectNormal);
                v3f rayParallel = (f32)(-sqrt( ABS_VALUE(1 - norm_squared(rayPerpendicular)))) * intersectNormal;
                
                newRayDir = rayPerpendicular + rayParallel;
            }
            
            Ray newRay = Ray(intersectPoint, newRayDir, false);
            
            v4f rayColour = cast_ray(&newRay, world, maxDepth - 1);
            resultColour = hadamard(material->colour, rayColour);
        }
    }
    else
    {
        // if no collisions we draw a simple gradient
        f32 ratio = 0.5f*(ray->dir.y + 1.0f);
        resultColour = (1.0f - ratio)*COLOUR_WHITE + ratio*v4f(0.5f, 0.8f, 0.9f);
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
    
    Image image = {};
    image.width = IMAGE_WIDTH;
    image.height = IMAGE_HEIGHT;
    image.pixels = (v4f*)memory_alloc(sizeof(v4f)*image.width*image.height);
    
    fill_image(&image, COLOUR_BLACK);
    
    printf("Setting up rendering scene...\n");
    
    World world = {};
    
    Material glassMaterial = create_dialectric_material(1.5f);
    Material groundMaterial = create_diffuse_material(v4f(0.8f, 0.8f, 0.0f));
    Material blueDiffuseMaterial = create_diffuse_material(v4f(0.1f, 0.2f, 0.5f));
    Material goldMetalMaterial = create_metal_material(v4f(0.8f, 0.6f, 0.2f), 0.0f);
    
    world.add_plane(v3f(0.0f, 1.0f, 0.0f), -2.0f, &groundMaterial);
    world.add_sphere(v3f(-1.0f, 0.0f, -1.0f), 0.5f, &glassMaterial);
    world.add_sphere(v3f(-1.0f, 0.0f, -1.0f), -0.45f, &glassMaterial);
    world.add_sphere(v3f(0.0f, 0.0f, -1.0f), 0.5f, &blueDiffuseMaterial);
    world.add_sphere(v3f(1.0f, 0.0f, -1.0f), 0.5f, &goldMetalMaterial);
    
    Camera  camera = Camera(v3f(-2.0f, 2.0f, 1.0f), 90, (f32)image.width/image.height);
    camera.focalLength = 1.0f;
    camera.set_target(v3f(0.0f, 0.0f, -1.0f));
    
    f32 pixelSize = camera.image_plane_width()/image.width;
    
    printf("Ray-tracing begins...\n");
    u32 finishedPercent = 0;
    
    f32 u = 0.0f, v = 0.0f; // offsets on the image plane from the top left point
    for (u32 pixelY = 0; pixelY < image.height; ++pixelY)
    {
        u = 0.0f;
        
        for (u32 pixelX = 0; pixelX < image.width; ++pixelX)
        {
            v4f pixelColour = v4f();
            
            for (u32 sampleIndex = 0; sampleIndex < SAMPLES_PER_PIXEL; ++sampleIndex)
            {
                f32 uOffset = random_f32()*pixelSize;
                f32 vOffset = random_f32()*pixelSize;
                
                Ray ray = camera.get_ray(u + uOffset, v + vOffset);
                pixelColour += cast_ray(&ray, &world, MAX_RAY_DEPTH);
            }
            
            set_pixel(&image, pixelX, pixelY, pixelColour/SAMPLES_PER_PIXEL);
            
            u += pixelSize;
        }
        
        v += pixelSize;
        
        // printing progress to console
        static const u32 TOTAL_ROWS = image.height;
        u32 currentPercent = (u32)((f32)pixelY/TOTAL_ROWS * 100.0f);
        if (currentPercent > finishedPercent)
        {
            finishedPercent = currentPercent;
            printf("%d%%\n", currentPercent);
        }
    }
    
    printf("Ray-tracing finished!\n");
    printf("Writing output to file: %s\n", fileName);
    
    write_image_to_bmp(fileName, &image);
    
    printf("File output complete. Program finished.\n");
    
    memory_free(fileName);
    memory_free(image.pixels);
    
    return 0;
}