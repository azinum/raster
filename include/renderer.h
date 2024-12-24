// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H

typedef enum Blend {
  BLEND_NONE,
  BLEND_ADD,

  MAX_BLEND_MODE,
} Blend;

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
#define COLOR_RGB(R, G, B)     ((Color) { .r = R, .g = G, .b = B, .a = 0xff, })
#define RECT(X, Y, W, H) (Rect) { .x = X, .y = Y, .w = W, .h = H, }

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height);
void renderer_set_blend_mode(Blend mode);
void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color);
void render_fill_rect_gradient(i32 x, i32 y, i32 w, i32 h, Color color_start, Color color_end, v2 gradient_start, v2 gradient_end);
void render_line(i32 x1, i32 y1, i32 x2, i32 y2, Color color);
void renderer_set_clear_color(Color color);
void render_clear(void);
void render_post(void);

#endif // _RENDERER_H
