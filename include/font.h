// font.h

#ifndef _FONT_H
#define _FONT_H

typedef struct Font {
  u8* glyphs;
  i32 count;
  i32 width;
  i32 height;
} Font;

Color font_get_color(Font* font, i32 x, i32 y, i32 code);
u8* font_get_glyph(Font* font, i32 code);

#endif // _FONT_H
