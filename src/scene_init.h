#ifndef SCENE_INIT_H
#define SCENE_INIT_H

// Generates a similar scene to the cover of the "Ray Tracing in One Weekend" book cover
// there are a bunch of random small spheres with random materials on a ground plane
// additionally there are 3 larger spheres, one with a glass material, and two with metal materials of different roughness
void init_test_scene_1(World* world, Camera* camera, f32 aspectRatio);

// Creates a scene with thousands of spheres arranged in a cube shape, surrounded by a couple walls and a glass floor
// each sphere has a random size, material, and colour (within a specific range)
void init_test_scene_2(World* world, Camera* camera, f32 aspectRatio);

// TODO: testing this out!
void init_test_scene_3(World* world, Camera* camera, f32 aspectRatio);

#endif //SCENE_INIT_H
