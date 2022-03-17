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
    RenderObject list[4192];
    u32 count;
    
    RenderObject* add_sphere(v3f pos, f32 radius, Material* material);
    RenderObject* add_plane(v3f normal, f32 d, Material* material);
};

#endif //RENDER_WORLD_H
