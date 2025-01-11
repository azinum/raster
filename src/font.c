// font.c

Color font_get_color(Font* font, i32 x, i32 y, i32 code) {
  if (code >= 0 && code < font->count) {
    x = CLAMP(x, 0, font->width - 1);
    y = CLAMP(y, 0, font->height - 1);
    u8* glyph = font_get_glyph(font, code);
    glyph += y * font->width + x;
    if (*glyph) {
      return COLOR_RGB(255, 255, 255);
    }
  }
  return COLOR_RGB(255, 0, 255);
}

inline u8* font_get_glyph(Font* font, i32 code) {
  return &font->glyphs[code * sizeof(u8) * font->width * font->height];
}
