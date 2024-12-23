// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H


typedef union Color {
  u32 value;
  struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
  };
} Color;

typedef struct Rect {
  i32 x;
  i32 y;
  i32 w;
  i32 h;
} Rect;

typedef union PackedRect8 {
  i32 value;
  struct {
    i8 x;
    i8 y;
    i8 w;
    i8 h;
  };
} PackedRect8;

typedef union PackedRect16 {
  i32 value_low;
  i32 value_high;
  struct {
    i16 x;
    i16 y;
    i16 w;
    i16 h;
  };
} PackedRect16;

#define COLOR_RGBA(R, G, B, A) ((Color) { .r = R, .g = G, .b = B, .a = A, })

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height);
void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color);
void renderer_set_clear_color(Color color);
void render_clear(void);

#endif // _RENDERER_H
