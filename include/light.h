// light.h

#ifndef _LIGHT_H
#define _LIGHT_H

typedef enum Light_type {
  LIGHT_POINT,
  LIGHT_SPOT,

  MAX_LIGHT_TYPE,
} Light_type;

typedef struct Light {
  v3 pos;
  f32 strength;
  f32 radius;
  f32 ambience;
  Light_type type;
} Light;

Light light_create(v3 pos, f32 strength, f32 radius);

#endif // _LIGHT_H
