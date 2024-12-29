// texture.c

inline Color texture_get_pixel(const Texture* const texture, const i32 x, const i32 y) {
  return texture->data[((y * texture->width) + x) % (texture->width * texture->height)];
}
