// texture.c

inline Color texture_get_pixel(const Texture* const texture, const i32 x, const i32 y) {
  return texture->data[((y * texture->width) + x) % (texture->width * texture->height)];
}

inline Color texture_get_pixel_wrapped(const Texture* const texture, const i32 x, const i32 y) {
  i32 x_coord = x % texture->width;
  i32 y_coord = y % texture->height;
  return texture_get_pixel(texture, x_coord, y_coord);
}
