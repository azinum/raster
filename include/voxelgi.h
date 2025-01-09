// voxelgi.h
// experimental voxel based global illumination

#ifndef _VOXELGI_H
#define _VOXELGI_H

typedef struct Vsample {
  v3 normal;
  f32 weight;
} Vsample;

typedef struct Voxelgi {
  v3 pos;
  i32 xsize;
  i32 ysize;
  i32 zsize;
  Vsample* samples;
  f32 contrib_speed;
  f32 decay_speed;
  f32 k1;
  f32 k2;
  f32 k3;
  f32 dist;

  i32 iterations;
  i32 max_iterations; // max iterations per frame
  i32 x;
  i32 y;
  i32 z;

  i32 voxel_update_tries;
  i32 voxel_update_interval;
  bool random_sampling;
  bool use_sampling_seed;
  Random sampling_seed;
} Voxelgi;

Voxelgi voxelgi_init(v3 pos, i32 xsize, i32 ysize, i32 zsize, Vsample* samples);
void voxelgi_update(Voxelgi* gi, f32 dt);
void voxelgi_update_voxel(Voxelgi* gi, v3 pos, v3 normal, f32 weight, f32 dt);
f32 voxelgi_get_voxel_weight(Voxelgi* gi, v3 pos);
void voxelgi_render(Voxelgi* gi);

#endif // _VOXELGI_H
