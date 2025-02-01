// texture.h

#ifndef _TEXTURE_H
#define _TEXTURE_H

typedef union Color32 {
  u32 value;
  struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
  };
} Color32;

typedef Color32 Color;

#define COLOR_RGBA(R, G, B, A) ((Color) { .r = R, .g = G, .b = B, .a = A, })
#define COLOR_RGB(R, G, B)     ((Color) { .r = R, .g = G, .b = B, .a = 0xff, })

typedef struct Texture {
  Color* data;
  u32 width;
  u32 height;
} Texture;

Color texture_get_pixel(const Texture* const texture, const i32 x, const i32 y);
Color texture_get_pixel_wrapped(const Texture* const texture, const u32 x, const u32 y);

#endif // _TEXTURE_H
