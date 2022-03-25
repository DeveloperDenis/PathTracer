#ifndef IMAGE_H
#define IMAGE_H

struct Image
{
    v4f* pixels;
    u32 width;
    u32 height;
};

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

#endif //IMAGE_H
