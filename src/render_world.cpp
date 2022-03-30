#include "render_world.h"

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

/*
* Material Functions
*/

Material Material::diffuse(v4f colour)
{
    Material result = {};
    result.type = Material::Type::DIFFUSE;
    result.colour = colour;
    return result;
}

Material Material::metal(v4f colour, f32 roughness)
{
    Material result = {};
    result.type = Material::Type::METAL;
    result.colour = colour;
    result.roughness = roughness;
    return result;
}

Material Material::dialectric(f32 refractiveIndex)
{
    Material result = {};
    result.type = Material::Type::DIALECTRIC;
    result.colour = Colour::WHITE; // TODO: do dialectrics always have no attenuation?
    result.n = refractiveIndex;
    return result;
}

/*
* Render Object Functions
*/

v3f SphereObject::pos(f32 time)
{
    return sphere.pos + velocity*time;
}

Rect3f SphereObject::get_bounding_box(f32 startTime, f32 endTime)
{
    Sphere startSphere = sphere;
    startSphere.pos += velocity*startTime;
    
    Sphere endSphere = sphere;
    endSphere.pos += velocity*endTime;
    
    Rect3f startBox = bounding_box(startSphere);
    Rect3f endBox = bounding_box(endSphere);
    
    return bounding_box(startBox, endBox);
}

/*
* World Functions
*/

SphereObject* World::add_sphere(v3f pos, f32 radius, Material* material, v3f velocity)
{
    assert(material);
    assert(objectCount < ARRAY_LENGTH(objects));
    
    if (!material || objectCount >= ARRAY_LENGTH(objects))
        return 0;
    
    SphereObject* object = objects + objectCount;
    *object = {};
    
    object->sphere.pos = pos;
    object->sphere.radius = radius;
    object->material = *material;
    object->velocity = velocity;
    
    ++objectCount;
    
    return object;
}

PlaneObject* World::add_plane(v3f normal, f32 d, Material* material)
{
    assert(material);
    assert(planeCount < ARRAY_LENGTH(planes));
    
    if (!material || planeCount >= ARRAY_LENGTH(planes))
        return 0;
    
    PlaneObject* object = planes + planeCount;
    *object = {};
    object->plane.normal = normal;
    object->plane.offset = d;
    object->material = *material;
    
    ++planeCount;
    
    return object;
}