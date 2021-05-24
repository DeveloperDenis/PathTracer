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

#define ASPECT_RATIO (16.0f/9.0f)
#define NUM_THREADS 16

#define RAY_TRACER_QUALITY 1

#if RAY_TRACER_QUALITY == 1
#define SAMPLES_PER_PIXEL 150
#define MAX_RAY_DEPTH 25
#define IMAGE_WIDTH 1280
#else
#define SAMPLES_PER_PIXEL 16
#define MAX_RAY_DEPTH 5
#define IMAGE_WIDTH 640
#endif

struct Colour
{
    static const v4f BLACK;
    static const v4f GREY;
    static const v4f SILVER;
    static const v4f WHITE;
    static const v4f RED;
    static const v4f BROWN;
    static const v4f ORANGE;
    static const v4f YELLOW;
    static const v4f GREEN;
    static const v4f DARK_GREEN;
    static const v4f TEAL;
    static const v4f BLUE;
    static const v4f INDIGO;
    static const v4f VIOLET;
    static const v4f PINK;
    static const v4f MAROON;
    static const v4f LAVENDER;
    static const v4f CYAN;
    static const v4f GOLD;
};

const v4f Colour::BLACK = v4f(0, 0, 0);
const v4f Colour::GREY = v4f(.5f, .5f, .5f);
const v4f Colour::SILVER = v4f(212.f/255, 212.f/255, 212.f/255);
const v4f Colour::WHITE = v4f(1, 1, 1);
const v4f Colour::RED = v4f(209.f/255, 42.f/255, 42.f/255);
const v4f Colour::BROWN = v4f(130.f/255, 108.f/255, 78.f/255);
const v4f Colour::ORANGE = v4f(1.0f, 156.f/255, 56.f/255);
const v4f Colour::YELLOW = v4f(1.0f, 243.f/255, 107.f/255);
const v4f Colour::GREEN = v4f(50.f/255, 161.f/255, 68.f/255);
const v4f Colour::DARK_GREEN = v4f(58.f/255, 89.f/255, 65.f/255);
const v4f Colour::TEAL = v4f(54.f/255, 158.f/255, 132.f/255);
const v4f Colour::BLUE = v4f(41.f/255, 85.f/255, 196.f/255);
const v4f Colour::INDIGO = v4f(45.f/255, 40.f/255, 168.f/255);
const v4f Colour::VIOLET = v4f(164.f/255, 86.f/255, 219.f/255);
const v4f Colour::PINK = v4f(235.f/255, 131.f/255, 231.f/255);
const v4f Colour::MAROON = v4f(117.f/255, 39.f/255, 53.f/255);
const v4f Colour::LAVENDER = v4f(219.f/255, 196.f/255, 245.f/255);
const v4f Colour::CYAN = v4f(179.f/255, 247.f/255, 1.0f);
const v4f Colour::GOLD = v4f(1.0f, 210.f/255, 8.f/255);

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
    RenderObject list[512];
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
    result.colour = Colour::WHITE; // TODO: do dialectrics always have no attenuation?
    result.n = refractiveIndex;
    return result;
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
        return Colour::BLACK;
    
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
                resultColour = Colour::BLACK;
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
        resultColour = (1.0f - ratio)*Colour::WHITE + ratio*v4f(0.7f, 0.8f, 0.9f);
    }
    
    return resultColour;
}

struct RayTracerBatch
{
    u32 startY;
    u32 endY;
    
    Image* outputImage;
    
    Camera* camera;
    World* world;
};

// TODO: I should rework all my random functions so that they don't rely on 'rand' anymore since those functions
// are not recommended for multi-threaded programs. Look into the C++ random library instead.
DWORD run_ray_tracer(void* data)
{
    RayTracerBatch* batchData = (RayTracerBatch*)data;
    
    for (u32 pixelY = batchData->startY; pixelY < batchData->endY; ++pixelY)
    {
        for (u32 pixelX = 0; pixelX < batchData->outputImage->width; ++pixelX)
        {
            v4f pixelColour = v4f();
            
            for (u32 sampleIndex = 0; sampleIndex < SAMPLES_PER_PIXEL; ++sampleIndex)
            {
                f32 u = (pixelX + random_f32())/batchData->outputImage->width;
                f32 v = (pixelY - random_f32())/batchData->outputImage->height;
                
                Ray ray = batchData->camera->get_ray(u, v);
                pixelColour += cast_ray(&ray, batchData->world, MAX_RAY_DEPTH);
            }
            
            set_pixel(batchData->outputImage, pixelX, pixelY, pixelColour/SAMPLES_PER_PIXEL);
        }
    }
    
    return 0;
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
    image.height = (u32)(image.width/ASPECT_RATIO);
    image.pixels = (v4f*)memory_alloc(sizeof(v4f)*image.width*image.height);
    
    fill_image(&image, Colour::BLACK);
    
    printf("Setting up rendering scene...\n");
    
    World world = {};
    
    // adding a bunch of materials
    
    Material materialList[32] = {};
    u32 numMaterials = 0;
    materialList[numMaterials++] = create_dialectric_material(1.5f);
    materialList[numMaterials++] = create_metal_material(Colour::GOLD, 0.2f);
    materialList[numMaterials++] = create_metal_material(Colour::SILVER, 0.01f);
    materialList[numMaterials++] = create_diffuse_material(Colour::WHITE);
    materialList[numMaterials++] = create_diffuse_material(Colour::RED);
    materialList[numMaterials++] = create_diffuse_material(Colour::ORANGE);
    materialList[numMaterials++] = create_diffuse_material(Colour::YELLOW);
    materialList[numMaterials++] = create_diffuse_material(Colour::GREEN);
    materialList[numMaterials++] = create_diffuse_material(Colour::BLUE);
    materialList[numMaterials++] = create_diffuse_material(Colour::INDIGO);
    materialList[numMaterials++] = create_diffuse_material(Colour::VIOLET);
    materialList[numMaterials++] = create_diffuse_material(Colour::PINK);
    materialList[numMaterials++] = create_diffuse_material(Colour::MAROON);
    materialList[numMaterials++] = create_diffuse_material(Colour::LAVENDER);
    materialList[numMaterials++] = create_diffuse_material(Colour::CYAN);
    materialList[numMaterials++] = create_diffuse_material(Colour::TEAL);
    materialList[numMaterials++] = create_diffuse_material(Colour::DARK_GREEN);
    materialList[numMaterials++] = create_diffuse_material(Colour::BROWN);
    
    // creating the scene to render!
    
    world.add_plane(v3f(0.0f, 1.0f, 0.0f), 0.0f, materialList + 3);
    
    const u32 GRID_ROW_COUNT = 16;
    const f32 GRID_CELL_SIZE = 3.5f;
    const f32 MIN_RADIUS = 0.5f;
    const f32 MAX_RADIUS = 0.7f;
    
    for (u32 z = 0; z < GRID_ROW_COUNT; ++z)
    {
        for (u32 x = 0; x < GRID_ROW_COUNT; ++x)
        {
            f32 minX = -(f32)GRID_ROW_COUNT/2*GRID_CELL_SIZE + x*GRID_CELL_SIZE + GRID_CELL_SIZE*0.5f;
            f32 minZ = -(f32)GRID_ROW_COUNT/2*GRID_CELL_SIZE + z*GRID_CELL_SIZE + GRID_CELL_SIZE*0.5f;
            
            f32 sphereX = minX + random_f32(-0.5, 0.5)*GRID_CELL_SIZE*.7f;
            f32 sphereZ = minZ + random_f32(-0.5, 0.5)*GRID_CELL_SIZE*.7f;
            f32 sphereY = 0.55f;
            
            v3f pos = v3f(sphereX, sphereY, sphereZ);
            f32 radius = random_f32(MIN_RADIUS, MAX_RADIUS);
            
            u32 materialIndex = random_u32(4, numMaterials);
            f32 materialPick = random_f32();
            if (materialPick > 0.9f)
            {
                materialIndex = 0;
            }
            
            world.add_sphere(pos, radius, materialList + materialIndex);
        }
    }
    
    world.add_sphere(v3f(1.0f, 4.0f, 0.5f), 4.0f, materialList);
    world.add_sphere(v3f(-11.0f, 4.0f, -5.0f), 4.0f, materialList + 1);
    world.add_sphere(v3f(5.5f, 4.0f, 15.0f), 4.0f, materialList + 2);
    
    // setting up camera properties
    
    Camera  camera = Camera(v3f(-3.5f, 2.5f, 35), 35, (f32)image.width/image.height);
    camera.set_target(v3f(0, .5f, 0));
    camera.up = normalize(v3f(.2f, 10, .8f));
    camera.set_lens(0.3f, 35.0f);
    
    // start the ray tracing!
    
    printf("Ray-tracing begins...\n");
    
    u32 rowsPerThread = image.height/NUM_THREADS;
    HANDLE threadHandles[NUM_THREADS];
    RayTracerBatch* threadData[NUM_THREADS];
    
    for (u32 threadIndex = 0; threadIndex < NUM_THREADS; ++threadIndex)
    {
        threadData[threadIndex] = (RayTracerBatch*)memory_alloc(sizeof(RayTracerBatch));
        threadData[threadIndex]->outputImage = &image;
        threadData[threadIndex]->camera = &camera;
        threadData[threadIndex]->world = &world;
        threadData[threadIndex]->startY = threadIndex*rowsPerThread;
        threadData[threadIndex]->endY = threadData[threadIndex]->startY + rowsPerThread;
        
        if (threadIndex >= NUM_THREADS - 1)
            threadData[threadIndex]->endY = image.height;
        
        threadHandles[threadIndex] = CreateThread(0, 0, run_ray_tracer, threadData[threadIndex], 0, 0);
        assert(threadHandles[threadIndex]);
    }
    
    WaitForMultipleObjects(NUM_THREADS, threadHandles, TRUE, INFINITE);
    
    for (u32 i = 0; i < NUM_THREADS; ++i)
    {
        CloseHandle(threadHandles[i]);
        memory_free(threadData[i]);
    }
    
    // TODO: see if I can incorporate this into the multi-threaded version
#if 0
    u32 finishedPercent = 0;
    // printing progress to console
    static const u32 TOTAL_ROWS = image.height;
    u32 currentPercent = (u32)((f32)pixelY/TOTAL_ROWS * 100.0f);
    if (currentPercent > finishedPercent)
    {
        finishedPercent = currentPercent;
        printf("%d%%\n", currentPercent);
    }
#endif
    
    printf("Ray-tracing finished!\n");
    printf("Writing output to file: %s\n", fileName);
    
    write_image_to_bmp(fileName, &image);
    
    printf("File output complete. Program finished.\n");
    
    memory_free(fileName);
    memory_free(image.pixels);
    
    return 0;
}