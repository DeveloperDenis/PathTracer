#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

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
    
    // creating new materials
    static Material diffuse(v4f colour);
    static Material metal(v4f colour, f32 roughness = 0.0f);
    static Material dialectric(f32 refractiveIndex);
};

// TODO: Do i want each object to have it's own material, or do I want to have a master
// material list that each object has an index into?
struct SphereObject
{
    Material material;
    Sphere sphere;
    
    // used for objects that move during the render interval
    // NOTE: the position of all objects are assumed to be defined at time = 0.0, so all times past that will be affected by the velocity
    v3f velocity;
    
    SphereObject() : material({}), sphere({}) {}
    
    // get the object position at the given time
    v3f pos(f32 time = 0.0f);
    Rect3f get_bounding_box(f32 startTime = 0.0f, f32 endTime = 0.0f);
};

struct PlaneObject
{
    Material material;
    Plane plane;
    
    PlaneObject() : material({}), plane({}) {}
};

// TODO: perhaps also keep track of a material list?
struct World
{
    u32 objectCount;
    SphereObject objects[4192];
    
    u32 planeCount;
    PlaneObject planes[64];
    
    // defines the interval during which our rendering takes place
    f32 startTime;
    f32 endTime;
    
    SphereObject* add_sphere(v3f pos, f32 radius, Material* material, v3f velocity = v3f());
    PlaneObject* add_plane(v3f normal, f32 d, Material* material);
};

#endif //RENDER_WORLD_H
