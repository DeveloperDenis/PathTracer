#include "scene_init.h"

static void generate_random_sphere_grid(World* world, u32 numRows, u32 numCols, f32 yLevel = 0.0f, f32 minRadius = 0.1f, f32 maxRadius = 0.5f)
{
    f32 cellSize = maxRadius*2.0f;
    
    Material glassMaterial = Material::dialectric(1.42f);
    
    for (u32 row = 0; row < numRows; ++row)
    {
        for (u32 col = 0; col < numCols; ++col)
        {
            u32 materialChoice = random_u32(0, 100);
            
            f32 sphereSize = random_f32(minRadius, maxRadius);
            v3f spherePos = v3f(row*cellSize, yLevel + sphereSize, col*cellSize);
            
            Material sphereMaterial;
            
            v4f sphereColour = v4f(random_v3f());
            sphereColour.b = 1.0f;
            
            if (materialChoice < 50)
                sphereMaterial = Material::diffuse(sphereColour);
            else if (materialChoice < 90)
                sphereMaterial = Material::metal(sphereColour, random_f32());
            else
                sphereMaterial = glassMaterial;
            
            world->add_sphere(spherePos, sphereSize, &sphereMaterial);
        }
    }
}

void init_test_scene_1(World* world, Camera* camera, f32 aspectRatio)
{
    assert(world);
    assert(camera);
    
    // adding a bunch of materials
    
    Material materialList[32] = {};
    u32 numMaterials = 0;
    materialList[numMaterials++] = Material::dialectric(1.5f);
    materialList[numMaterials++] = Material::metal(Colour::GOLD, 0.2f);
    materialList[numMaterials++] = Material::metal(Colour::SILVER, 0.01f);
    materialList[numMaterials++] = Material::diffuse(Colour::WHITE);
    materialList[numMaterials++] = Material::diffuse(Colour::RED);
    materialList[numMaterials++] = Material::diffuse(Colour::ORANGE);
    materialList[numMaterials++] = Material::diffuse(Colour::YELLOW);
    materialList[numMaterials++] = Material::diffuse(Colour::GREEN);
    materialList[numMaterials++] = Material::diffuse(Colour::BLUE);
    materialList[numMaterials++] = Material::diffuse(Colour::INDIGO);
    materialList[numMaterials++] = Material::diffuse(Colour::VIOLET);
    materialList[numMaterials++] = Material::diffuse(Colour::PINK);
    materialList[numMaterials++] = Material::diffuse(Colour::MAROON);
    materialList[numMaterials++] = Material::diffuse(Colour::LAVENDER);
    materialList[numMaterials++] = Material::diffuse(Colour::CYAN);
    materialList[numMaterials++] = Material::diffuse(Colour::TEAL);
    materialList[numMaterials++] = Material::diffuse(Colour::DARK_GREEN);
    materialList[numMaterials++] = Material::diffuse(Colour::BROWN);
    
    // creating the scene to render!
    
    world->add_plane(v3f(0.0f, 1.0f, 0.0f), 0.0f, materialList + 3);
    
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
            
            world->add_sphere(pos, radius, materialList + materialIndex);
        }
    }
    
    world->add_sphere(v3f(1.0f, 4.0f, 0.5f), 4.0f, materialList);
    world->add_sphere(v3f(-11.0f, 4.0f, -5.0f), 4.0f, materialList + 1);
    world->add_sphere(v3f(5.5f, 4.0f, 15.0f), 4.0f, materialList + 2);
    
    // setting up camera properties
    
    *camera = Camera(v3f(-3.5f, 2.5f, 35), 35, aspectRatio);
    camera->set_target(v3f(0, .5f, 0));
    camera->up = normalize(v3f(.2f, 10, .8f));
    camera->set_lens(0.3f, 35.0f);
}

void init_test_scene_2(World* world, Camera* camera, f32 aspectRatio)
{
    assert(world);
    assert(camera);
    
    Material wallMaterial = Material::diffuse(Colour::LAVENDER);
    
    Material glassMaterial = Material::dialectric(1.42f);
    
    const u32 NUM_ROWS = 20;
    const u32 NUM_COLS = 20;
    const f32 MIN_RADIUS = 0.8f;
    const f32 MAX_RADIUS = 2.0f;
    
    const f32 CELL_SIZE = MAX_RADIUS*2.0f;
    const f32 CELL_Y_SPACING = 2.5f;
    
    // NOTE: I can't put up three walls at once because it eats away all the light
    world->add_plane(v3f(0.0f, 0.0f, 1.0f), -3.0f, &wallMaterial); // back wall
    //world->add_plane(v3f(1.0f, 0.0f, 0.0f), -5.0f, &testMaterial); // left wall
    world->add_plane(v3f(-1.0f, 0.0, 0.0f), -((f32)NUM_ROWS*(f32)CELL_SIZE) - 3.0f, &wallMaterial); // right wall
    world->add_plane(v3f(0.0f, 1.0f, 0.0f), -0.1f, &glassMaterial); // floor
    
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, 0.0f, MIN_RADIUS, MAX_RADIUS);
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, CELL_SIZE + CELL_Y_SPACING, MIN_RADIUS, MAX_RADIUS);
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, CELL_SIZE*2.0f + CELL_Y_SPACING*2.0f, MIN_RADIUS, MAX_RADIUS);
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, CELL_SIZE*3.0f + CELL_Y_SPACING*3.0f, MIN_RADIUS, MAX_RADIUS);
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, CELL_SIZE*4.0f + CELL_Y_SPACING*4.0f, MIN_RADIUS, MAX_RADIUS);
    generate_random_sphere_grid(world, NUM_ROWS, NUM_COLS, CELL_SIZE*5.0f + CELL_Y_SPACING*5.0f, MIN_RADIUS, MAX_RADIUS);
    
    v3f cameraPos = v3f(-5.0f, 3.0f, NUM_ROWS*CELL_SIZE*1.2f);
    *camera = Camera(cameraPos, 50.0f, aspectRatio);
    camera->set_target(v3f((f32)NUM_ROWS*0.5f*CELL_SIZE, 30.0f, (f32)NUM_COLS*0.5f*CELL_SIZE));
    camera->set_lens(1.0f, 40.0f);
}

void init_test_scene_3(World* world, Camera* camera, f32 aspectRatio)
{
    assert(world);
    assert(camera);
    
    Material floorMaterial = Material::diffuse(Colour::RED);
    world->add_plane(v3f(0.0f, 1.0f, 0.0f), 0.0f, &floorMaterial);
    
    Material materials[] = 
    {
        Material::diffuse(Colour::PINK),
        Material::diffuse(Colour::YELLOW)
    };
    
    v3f sphereVelocity = v3f(0.5f, 0.0f, 0.0f);
    
    world->startTime = 0.0f;
    world->endTime = 1.0f;
    
    world->add_sphere(v3f(-1.0f, 2.0f, -2.0f), 0.5f, materials, sphereVelocity);
    world->add_sphere(v3f(2.0f, 1.01f, -3.5f), 1.0f, materials + 1, v3f(0.0f, 0.1f, 0.0f));
    
    // set up camera
    
    v3f cameraPos = v3f(0.0f, 2.0f, 2.0f);
    *camera = Camera(cameraPos, 45.0f, aspectRatio);
}
