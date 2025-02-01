// texture.c

inline Color texture_get_pixel(const Texture* const texture, const i32 x, const i32 y) {
  return texture->data[((y * texture->width) + x) % (texture->width * texture->height)];
}

inline Color texture_get_pixel_wrapped(const Texture* const texture, const u32 x, const u32 y) {
  const u32 x_coord = x % texture->width;
  const u32 y_coord = y % texture->height;
  return texture->data[(y_coord * texture->width) + x_coord];
}
