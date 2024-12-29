// texture.h

#ifndef _TEXTURE_H
#define _TEXTURE_H

struct Color;

typedef struct Texture {
  struct Color* data;
  u32 width;
  u32 height;
} Texture;

#endif // _TEXTURE_H
