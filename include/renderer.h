// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H

typedef enum Blend {
  BLEND_NONE,
  BLEND_ADD,

  MAX_BLEND_MODE,
} Blend;

typedef enum Render_target {
  RENDER_TARGET_COLOR,
  RENDER_TARGET_CLEAR,

  MAX_RENDER_TARGET,
} Render_target;

typedef union Color32 {
  u32 value;
  struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
  };
} Color32;

typedef Color32 Color;

#define COLOR_RGBA(R, G, B, A) ((Color) { .r = R, .g = G, .b = B, .a = A, })
#define COLOR_RGB(R, G, B)     ((Color) { .r = R, .g = G, .b = B, .a = 0xff, })

typedef union Rect {
  struct {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
  };
  struct {
    i32 x1;
    i32 y1;
    i32 x2;
    i32 y2;
  };
} Rect;

typedef union PackedRect8 {
  i32 value;
  struct {
    i8 x;
    i8 y;
    i8 w;
    i8 h;
  };
  struct {
    i8 x1;
    i8 y1;
    i8 x2;
    i8 y2;
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
  struct {
    i16 x1;
    i16 y1;
    i16 x2;
    i16 y2;
  };
} PackedRect16;

#define RECT(X, Y, W, H) (Rect) { .x = X, .y = Y, .w = W, .h = H, }

void renderer_init(Color* color_buffer, Color* clear_buffer, u32 width, u32 height);
void renderer_set_blend_mode(Blend mode);
void renderer_set_render_target(Render_target render_target);
void render_rect(i32 x, i32 y, i32 w, i32 h, Color color);
void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color);
void render_fill_rect_gradient(i32 x, i32 y, i32 w, i32 h, Color color_start, Color color_end, v2 gradient_start, v2 gradient_end);
void render_line(i32 x1, i32 y1, i32 x2, i32 y2, Color color);
void render_fill_triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color);
void render_fill_circle(i32 x, i32 y, i32 r, Color color);
void render_mesh(Mesh* mesh, v3 position, v3 size, v3 rotation);
void renderer_set_clear_color(Color color);
void renderer_begin(void);
void render_post(void);
void render_clear(void);
i32 renderer_get_num_primitives(void);
i32 renderer_get_num_primitives_culled(void);

#endif // _RENDERER_H
