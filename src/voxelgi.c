// voxelgi.c

static Vsample* voxelgi_get_sample(Voxelgi* gi, i32 x, i32 y, i32 z);
static Vsample* voxelgi_get_sample_within_bounds(Voxelgi* gi, i32 x, i32 y, i32 z);

// x = left/right
// y = up/down
// z = front/back
inline Vsample* voxelgi_get_sample(Voxelgi* gi, i32 x, i32 y, i32 z) {
#ifdef DEBUG_OUT_OF_BOUNDS
  ASSERT(
    x >= 0 && x < gi->xsize &&
    y >= 0 && y < gi->ysize &&
    z >= 0 && z < gi->zsize
  );
#endif
  return &gi->samples[(gi->xsize * gi->zsize * y) + (gi->xsize * z) + x];
}

Vsample* voxelgi_get_sample_within_bounds(Voxelgi* gi, i32 x, i32 y, i32 z) {
  if (
    x >= 0 && x < gi->xsize &&
    y >= 0 && y < gi->ysize &&
    z >= 0 && z < gi->zsize
  ) {
    return &gi->samples[(gi->xsize * gi->zsize * y) + (gi->xsize * z) + x];
  }
  return NULL;
}

Voxelgi voxelgi_init(v3 pos, i32 xsize, i32 ysize, i32 zsize, Vsample* samples) {
  Voxelgi gi = (Voxelgi) {
    .pos = pos,
    .xsize = xsize,
    .ysize = ysize,
    .zsize = zsize,
    .samples = samples,
    .contrib_speed = 35.0f,
    .decay_speed = 55.0f,
    .k1 = 10.0f,
    .k2 = 20.0f,
    .k3 = 30.0f,
    .dist = 1.2f,

    .max_iterations = 100,
    .iterations = 0,
    .x = 0,
    .y = 0,
    .z = 0,

    .voxel_update_tries = 0,
    .voxel_update_interval = 1,
  };
  for (i32 y = 0; y < ysize; ++y) {
    for (i32 z = 0; z < zsize; ++z) {
      for (i32 x = 0; x < xsize; ++x) {
        Vsample* sample = voxelgi_get_sample(&gi, x, y, z);
#if 0
        sample->normal = V3(
          random_f32() - random_f32(),
          random_f32() - random_f32(),
          random_f32() - random_f32()
        );
        sample->weight = random_f32();
#else
        sample->normal = V3(0, 0, 0);
        sample->weight = 0;
#endif
      }
    }
  }
  return gi;
}

void voxelgi_update(Voxelgi* gi, f32 dt) {
  gi->x %= gi->xsize;
  gi->y %= gi->ysize;
  gi->z %= gi->zsize;
  f32 iterations = 0;
  gi->voxel_update_tries = random_number();

  for (; gi->y < gi->ysize; ++gi->y) {
    for (; gi->z < gi->zsize; ++gi->z) {
      for (; gi->x < gi->xsize; ++gi->x) {
        // if (gi->iterations >= gi->max_iterations) {
        //   gi->iterations = 0;
        //   return;
        // }
        gi->iterations += 1;
        iterations += 1;
        Vsample* sample = voxelgi_get_sample(gi, gi->x, gi->y, gi->z);
        v3 sample_pos = V3(gi->x, gi->y, gi->z);
        f32 fsum = 0;
        for (i32 sy = -2; sy < 2; ++sy) {
          for (i32 sz = -2; sz < 2; ++sz) {
            for (i32 sx = -2; sx < 2; ++sx) {
              if (sx == 0 && sy == 0 && sz == 0) {
                continue;
              }
              Vsample* n = voxelgi_get_sample_within_bounds(gi, gi->x + sx, gi->y + sy, gi->z + sz);
              if (!n) {
                continue;
              }
              v3 np = V3(gi->x + sx, gi->y + sy, gi->z + sz);
              v3 np_local = V3(sx, sy, sz);
              v3 np_local_normalized = v3_normalize(np_local);
              v3 d1 = v3_normalize(V3_OP(sample->normal, n->normal, +));
              f32 distance = v3_length_square(np_local) + v3_length_square(d1);
              f32 contrib = CLAMP(1 / (1 + ((distance + gi->dist)/(gi->k1*sample->weight * gi->k2*sample->weight * gi->k3*sample->weight))), 0, 1);
              contrib *= CLAMP(v3_dot(sample->normal, np_local_normalized), 0, 1);
              fsum += contrib;
              f32 f = contrib;
              n->weight = lerp(n->weight, sample->weight, f * gi->contrib_speed * dt);
              n->normal = V3(
                lerp(n->normal.x, sample->normal.x, f * gi->contrib_speed * dt),
                lerp(n->normal.y, sample->normal.y, f * gi->contrib_speed * dt),
                lerp(n->normal.z, sample->normal.z, f * gi->contrib_speed * dt)
              );
            }
          }
        }
        fsum /= iterations;
        sample->weight = lerp(sample->weight, 0, fsum * gi->decay_speed * dt);
      }
      gi->x = 0;
    }
    gi->z = 0;
  }
}

void voxelgi_update_voxel(Voxelgi* gi, v3 pos, v3 normal, f32 weight) {
  gi->voxel_update_tries += 1;

  if ((gi->voxel_update_tries % gi->voxel_update_interval) != 0) {
    return;
  }
  i32 x = gi->pos.x + pos.x;
  i32 y = gi->pos.y + pos.y;
  i32 z = gi->pos.z + pos.z;
  Vsample* sample = voxelgi_get_sample_within_bounds(gi, x, y, z);
  if (sample) {
#if 0
    sample->normal = normal;
    sample->weight = weight;
#else
    sample->normal = V3(
      lerp(normal.x, sample->normal.x, weight),
      lerp(normal.y, sample->normal.y, weight),
      lerp(normal.z, sample->normal.z, weight)
    );
    sample->weight = lerp(sample->weight, weight, 0.8f); // TODO: pass dt
#endif
  }
}

f32 voxelgi_get_voxel_weight(Voxelgi* gi, v3 pos) {
  i32 x = gi->pos.x + pos.x;
  i32 y = gi->pos.y + pos.y;
  i32 z = gi->pos.z + pos.z;
  Vsample* sample = voxelgi_get_sample_within_bounds(gi, x, y, z);
  if (sample) {
    return sample->weight;
  }
  return 0;
}

void voxelgi_render(Voxelgi* gi) {
  for (i32 y = 0; y < gi->ysize; ++y) {
    for (i32 z = 0; z < gi->zsize; ++z) {
      for (i32 x = 0; x < gi->xsize; ++x) {
        Vsample* sample = voxelgi_get_sample(gi, x, y, z);
        u8 c = sample->weight * UINT8_MAX;
        Color color = COLOR_RGB(
          c * ABS(f32, sample->normal.x),
          c * ABS(f32, sample->normal.y),
          c * ABS(f32, sample->normal.z)
        );
        render_point_3d(V3_OP(V3(x, y, z), gi->pos, -), color);
      }
    }
  }
}
