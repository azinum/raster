// light.c

Light light_create(v3 pos, f32 strength, f32 radius) {
  return (Light) {
    .pos = pos,
    .strength = strength,
    .radius = radius,
    .ambience = LIGHT_AMBIENCE,
    .type = LIGHT_POINT,
  };
}

f32 light_calculate_contribution(Light light, v3 pos, v3 normal) {
  f32 result = 0;

  v3 light_delta = V3_OP(light.pos, pos, -);
  v3 light_normalized = v3_normalize(light_delta);
  f32 distance = v3_length_square(light_delta);
  f32 attenuation = 1.0f / (1.0f + (distance)/(light.radius*light.radius*light.radius));
  result = v3_dot(normal, light_normalized) * attenuation * light.strength;
  result = CLAMP(result, light.ambience, 1);

  return result;
}
