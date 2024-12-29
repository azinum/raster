// light.h

#ifndef _LIGHT_H
#define _LIGHT_H

typedef struct Light {
  v3 pos;
  f32 strength;
  f32 radius;
  f32 ambience;
} Light;

Light light_create(v3 pos, f32 strength, f32 radius);

#endif // _LIGHT_H
