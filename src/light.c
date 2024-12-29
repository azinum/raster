// light.c

Light light_create(v3 pos, f32 strength, f32 radius) {
  return (Light) {
    .pos = pos,
    .strength = strength,
    .radius = radius,
    .ambience = LIGHT_AMBIENCE,
  };
}
