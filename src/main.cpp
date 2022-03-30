#include <stdio.h>
#include <assert.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

#include "types.h"
#include "memory.h"
#include "strings.h"
#include "vectors.cpp"
#include "image.h"
#include "file_io.h"
#include "geometry.cpp"
#include "camera.cpp"
#include "render_world.cpp"
#include "scene_init.cpp"

#define FILE_EXT ".bmp"

#define ASPECT_RATIO (16.0f/9.0f)

// it seems like the sweet spot of block size might be between 16 & 32
#define BLOCK_SIZE 32
#define NUM_THREADS 16

#define RAY_TRACER_QUALITY 1

#if RAY_TRACER_QUALITY == 1
#define SAMPLES_PER_PIXEL 200
#define MAX_RAY_DEPTH 50
#define IMAGE_WIDTH 1920
#else
#define SAMPLES_PER_PIXEL 32
#define MAX_RAY_DEPTH 5
#define IMAGE_WIDTH 800
#endif

struct BVH
{
    Rect3f boundingBox;
    SphereObject* object; // only valid in the leaf nodes
    
    BVH* left;
    BVH* right;
};

static f32 intersection_test(Ray ray, BVH* bvh, f32 time, SphereObject** outObject)
{
    f32 tResult = F32_MAX;
    const f32 MIN_T = 0.001f;
    
    if (!bvh)
        return tResult;
    
    if (bvh->object) // reached a leaf node
    {
        *outObject = bvh->object;
        
        Sphere testSphere = bvh->object->sphere;
        testSphere.pos += time*bvh->object->velocity;
        
        tResult = intersection_test(ray, testSphere);
    }
    else if (hit_test(ray, bvh->boundingBox))
    {
        SphereObject* leftObject = 0;
        SphereObject* rightObject = 0;
        
        f32 tLeft = intersection_test(ray, bvh->left, time, &leftObject);
        f32 tRight = intersection_test(ray, bvh->right, time, &rightObject);
        
        if (tLeft < tRight && tLeft > MIN_T)
        {
            tResult = tLeft;
            *outObject = leftObject;
        }
        else if (tRight < tLeft && tRight > MIN_T)
        {
            tResult = tRight;
            *outObject = rightObject;
        }
    }
    
    return tResult;
}

static void sort_render_objects(SphereObject* list, u32 startIndex, u32 endIndex, u32 sortAxis)
{
    assert(list);
    assert(sortAxis >= 0 && sortAxis < 3);
    assert(startIndex < endIndex);
    
    if (endIndex - startIndex == 1)
        return;
    else if (endIndex - startIndex == 2)
    {
        if (list[startIndex].pos()[sortAxis] > list[startIndex + 1].pos()[sortAxis])
            SWAP(list[startIndex], list[startIndex + 1], SphereObject);
    }
    else
    {
        // just choose the first element as the pivot, though I could use any other method
        u32 pivotIndex = startIndex;
        
        u32 splitIndex = startIndex + 1;
        for (u32 i = startIndex + 1; i < endIndex; ++i)
        {
            if (list[i].pos()[sortAxis] < list[pivotIndex].pos()[sortAxis])
            {
                SWAP(list[i], list[splitIndex], SphereObject);
                ++splitIndex;
            }
        }
        
        SWAP(list[pivotIndex], list[splitIndex - 1], SphereObject);
        
        sort_render_objects(list, startIndex, splitIndex, sortAxis);
        
        // only sort the other half if the pivot wasn't the largest element in the list
        if (splitIndex != endIndex)
            sort_render_objects(list, splitIndex, endIndex, sortAxis);
    }
}

static BVH* build_bvh_tree(SphereObject* objects, u32 startIndex, u32 endIndex, f32 startTime = 0.0f, f32 endTime = 0.0f)
{
    u32 sortAxis = random_u32(0, 3);
    
    BVH* newNode = 0;
    
    if (endIndex - startIndex == 1)
    {
        newNode = (BVH*)memory_alloc(sizeof(BVH));
        newNode->object = objects + startIndex;
        newNode->boundingBox = objects[startIndex].get_bounding_box(startTime, endTime);
    }
    else
    {
        sort_render_objects(objects, startIndex, endIndex, sortAxis);
        
        u32 midIndex = (startIndex + endIndex)/2;
        
        BVH* leftNode = build_bvh_tree(objects, startIndex, midIndex, startTime, endTime);
        BVH* rightNode = build_bvh_tree(objects, midIndex, endIndex, startTime, endTime);
        
        newNode = (BVH*)memory_alloc(sizeof(BVH));
        newNode->left = leftNode;
        newNode->right = rightNode;
        newNode->boundingBox = bounding_box(leftNode->boundingBox, rightNode->boundingBox);
    }
    
    return newNode;
}

// calculates reflectance for a material using Schlick's Approximation
static f64 reflectance(f64 cosine, f64 refractRatio)
{
    f64 r0 = (1.0 - refractRatio) / (1.0 + refractRatio);
    r0 = r0*r0;
    
    return r0 + (1.0 - r0)*pow((1.0 - cosine), 5);
}

// returns colour of pixel after ray cast
static v4f cast_ray(Ray ray, World* world, BVH* bvh, u32 maxDepth = 1, f32 time = 0.0f)
{
    v4f resultColour = v4f();
    
    if (maxDepth <= 0)
        return Colour::BLACK;
    
    const f32 MIN_T = 0.001f;
    
    f32 tClosest = F32_MAX;
    v3f intersectNormal = v3f();
    v3f intersectPoint = v3f();
    Material* material = 0;
    
    // checking for intersection
    for (u32 i = 0; i < world->planeCount; ++i) // plane objects
    {
        f32 t = F32_MIN;
        
        Plane plane = world->planes[i].plane;
        t = intersection_test(ray, plane);
        
        if (t > MIN_T && t < tClosest)
        {
            tClosest = t;
            intersectPoint = ray.at(t);
            intersectNormal= plane.normal;
            material = &world->planes[i].material;
        }
    }
    
    SphereObject* testObject = 0;
    f32 t = intersection_test(ray, bvh, time, &testObject);
    if (t > MIN_T && t < tClosest)
    {
        assert(testObject);
        
        tClosest = t;
        
        Sphere testSphere = Sphere(testObject->sphere.pos + time*testObject->velocity, testObject->sphere.radius);
        
        intersectPoint = ray.at(t);
        intersectNormal = normalize(intersectPoint - testSphere.pos);
        
        material = &testObject->material;
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
            v4f rayColour = cast_ray(reflectRay, world, bvh, maxDepth - 1, time);
            
            // attenuate using the colour of the material
            resultColour = hadamard(material->colour, rayColour);
        }
        else if (material->type == Material::Type::METAL)
        {
            // the reflected ray is calculated assuming the surface is a perfect mirror
            v3f reflectedDir = reflect_direction(ray.dir, intersectNormal);
            
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
                v4f rayColour = cast_ray(reflectedRay, world, bvh, maxDepth - 1, time);
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
            if (dot(ray.dir, intersectNormal) > 0.0f) // if ray and normal in same direction
                refractRatio = 1.0f/refractRatio;
            
            f32 cosTheta = dot(-ray.dir, intersectNormal);
            f32 sinTheta = (f32)sqrt(1 - cosTheta*cosTheta);
            
            v3f newRayDir = v3f();
            
            bool internalReflection  = refractRatio * sinTheta > 1.0f;
            // using Schlick's Approximation
            bool shouldReflect = reflectance(cosTheta, refractRatio) > random_f64();
            
            if (internalReflection || shouldReflect)
            {
                // Refraction impossible, so the ray must reflect
                newRayDir = reflect_direction(ray.dir, intersectNormal);
            }
            else
            {
                // Refraction!
                
                v3f rayPerpendicular = (refractRatio)*(ray.dir + cosTheta*intersectNormal);
                v3f rayParallel = (f32)(-sqrt( ABS_VALUE(1 - norm_squared(rayPerpendicular)))) * intersectNormal;
                
                newRayDir = rayPerpendicular + rayParallel;
            }
            
            Ray newRay = Ray(intersectPoint, newRayDir, false);
            
            v4f rayColour = cast_ray(newRay, world, bvh, maxDepth - 1, time);
            resultColour = hadamard(material->colour, rayColour);
        }
    }
    else
    {
        // if no collisions we draw a simple gradient
        f32 ratio = 0.5f*(ray.dir.y + 1.0f);
        resultColour = (1.0f - ratio)*Colour::WHITE + ratio*v4f(0.7f, 0.8f, 0.9f);
    }
    
    return resultColour;
}

struct RayTracerBatch
{
    u32 startX, startY;
    u32 endX, endY;
    
    Image* outputImage;
    
    Camera* camera;
    World* world;
    BVH* bvh;
};

DWORD run_ray_tracer(void* data)
{
    RayTracerBatch* batchData = (RayTracerBatch*)data;
    
    for (u32 pixelY = batchData->startY; pixelY < batchData->endY; ++pixelY)
    {
        for (u32 pixelX = batchData->startX; pixelX < batchData->endX; ++pixelX)
        {
            v4f pixelColour = v4f();
            
            for (u32 sampleIndex = 0; sampleIndex < SAMPLES_PER_PIXEL; ++sampleIndex)
            {
                f32 rayTime = random_f32(batchData->world->startTime, batchData->world->endTime);
                
                f32 u = (pixelX + random_f32())/batchData->outputImage->width;
                f32 v = (pixelY - random_f32())/batchData->outputImage->height;
                
                Ray ray = batchData->camera->get_ray(u, v);
                pixelColour += cast_ray(ray, batchData->world, batchData->bvh, MAX_RAY_DEPTH, rayTime);
            }
            
            pixelColour = clamp(pixelColour/SAMPLES_PER_PIXEL, 0.0f, 1.0f);
            set_pixel(batchData->outputImage, pixelX, pixelY, pixelColour);
        }
    }
    
    return 0;
}

bool get_pixel_block(u32 blockIndex, u32 totalWidth, u32 totalHeight, u32* startX, u32* startY, u32* endX, u32* endY)
{
    u32 blocksPerCol = totalWidth/BLOCK_SIZE;
    u32 blocksPerRow = totalHeight/BLOCK_SIZE;
    
    if (blockIndex >= blocksPerCol * blocksPerRow)
        return false;
    
    u32 startBlockX = blockIndex % blocksPerCol;
    u32 startBlockY = blockIndex/blocksPerCol;
    
    *startX = startBlockX * BLOCK_SIZE;
    *startY = startBlockY * BLOCK_SIZE;
    
    if (startBlockX == blocksPerCol - 1)
        *endX = totalWidth;
    else
        *endX = *startX + BLOCK_SIZE;
    
    if (startBlockY == blocksPerRow - 1)
        *endY = totalHeight;
    else
        *endY = *startY + BLOCK_SIZE;
    
    return true;
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
    
    f32 aspectRatio = (f32)image.width/image.height;
    
    World world = {};
    Camera camera = {};
    
    init_test_scene_2(&world, &camera, aspectRatio);
    
    BVH* bvh = build_bvh_tree(world.objects, 0, world.objectCount, world.startTime, world.endTime);
    assert(bvh);
    
    // start the ray tracing!
    
    printf("Ray-tracing begins...\n");
    
    LARGE_INTEGER countsPerSecond = {};
    LARGE_INTEGER startTime = {};
    LARGE_INTEGER endTime = {};
    
    QueryPerformanceFrequency(&countsPerSecond);
    QueryPerformanceCounter(&startTime);
    
    // TODO: I think there is a problem with how I handle multi-threading? I can't tell most of the time because
    // of the regular amount of noise, but with a block size of 8 there appears to be weird artifacting
    
    // set up all the threads to run the initial batch of pixels
    
    u32 nextBlock = 0;
    
    HANDLE threadHandles[NUM_THREADS];
    RayTracerBatch* threadData[NUM_THREADS];
    
    for (nextBlock = 0; nextBlock < NUM_THREADS; ++nextBlock)
    {
        u32 threadIndex = nextBlock;
        
        threadData[threadIndex] = (RayTracerBatch*)memory_alloc(sizeof(RayTracerBatch));
        threadData[threadIndex]->outputImage = &image;
        threadData[threadIndex]->camera = &camera;
        threadData[threadIndex]->world = &world;
        threadData[threadIndex]->bvh = bvh;
        
        u32 x1, x2, y1, y2;
        
#if NUM_THREADS == 1
        x1 = 0;
        x2 = image.width;
        y1 = 0;
        y2 = image.height;
#else
        get_pixel_block(nextBlock, image.width, image.height, &x1, &y1, &x2, &y2);
#endif
        
        threadData[threadIndex]->startX = x1;
        threadData[threadIndex]->startY = y1;
        threadData[threadIndex]->endX = x2;
        threadData[threadIndex]->endY = y2;
        
        threadHandles[threadIndex] = CreateThread(0, 0, run_ray_tracer, threadData[threadIndex], 0, 0);
        assert(threadHandles[threadIndex]);
    }
    
    // when threads finish their work give them more, as long as there is more to give
    
    u32 runningThreads = NUM_THREADS;
    u32 finishedBlocks = 0;
    u32 totalBlocks = (image.width/BLOCK_SIZE) * (image.height/BLOCK_SIZE);
    
    while (runningThreads > 0)
    {
        DWORD result = WaitForMultipleObjects(runningThreads, threadHandles, FALSE, INFINITE);
        if (result == WAIT_FAILED)
            break;
        
        if (result >= WAIT_OBJECT_0 && result <= WAIT_OBJECT_0 + NUM_THREADS)
        {
            // TODO: it would be better to do a thread-pooling thing here, so either look into the win32 version,
            // or see if the CRT or C++ library make it easier
            u32 threadIndex = result - WAIT_OBJECT_0;
            CloseHandle(threadHandles[threadIndex]);
            
#if NUM_THREADS == 1
            memory_free(threadData[threadIndex]);
            threadData[threadIndex] = threadData[runningThreads - 1];
            threadHandles[threadIndex] = threadHandles[runningThreads - 1];
            --runningThreads;
#else
            u32 x1, y1, x2, y2;
            if (get_pixel_block(nextBlock, image.width, image.height, &x1, &y1, &x2, &y2))
            {
                threadData[threadIndex]->startX = x1;
                threadData[threadIndex]->startY = y1;
                threadData[threadIndex]->endX = x2;
                threadData[threadIndex]->endY = y2;
                
                threadHandles[threadIndex] = CreateThread(0, 0, run_ray_tracer, threadData[threadIndex], 0, 0);
                
                ++nextBlock;
            }
            else
            {
                memory_free(threadData[threadIndex]);
                threadData[threadIndex] = threadData[runningThreads - 1];
                threadHandles[threadIndex] = threadHandles[runningThreads - 1];
                --runningThreads;
            }
#endif
            
            ++finishedBlocks;
            printf("%f%%\n", ((f32)finishedBlocks/totalBlocks)*100.0f);
        }
    }
    
    QueryPerformanceCounter(&endTime);
    f64 timeElapsed = (endTime.QuadPart - startTime.QuadPart) / (f64)countsPerSecond.QuadPart;
    
    printf("Ray-tracing finished!\n");
    printf("Time elapsed: %f seconds\n", timeElapsed);
    printf("Writing output to file: %s\n", fileName);
    
    write_image_to_bmp(fileName, &image);
    
    printf("File output complete. Program finished.\n");
    
    memory_free(fileName);
    memory_free(image.pixels);
    
    return 0;
}