// renderer.c

typedef struct Renderer {
  Color* buffer;
  Color* clear_buffer;
  u32 width;
  u32 height;
  Blend blend_mode;
} Renderer;

static Renderer renderer;

static Color* renderer_get_pixel_addr(i32 x, i32 y);
static void renderer_draw_pixel(Color* pixel, Color color);
static bool renderer_normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect);
static Color lerp_color(Color a, Color b, f32 t);

Color* renderer_get_pixel_addr(i32 x, i32 y) {
  return &renderer.buffer[y * renderer.width + x];
}

void renderer_draw_pixel(Color* pixel, Color color) {
  switch (renderer.blend_mode) {
    case BLEND_NONE: {
      *pixel = color;
      break;
    }
    case BLEND_ADD: {
      pixel->value += color.value;
      pixel->a = color.a;
      break;
    }
    default:
      break;
  }
}

// clamp rect to boundary, occlude if not visible
bool renderer_normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect) {
  if (w <= 0) { return false; }
  if (h <= 0) { return false; }
  if (x >= renderer.width) { return false; }
  if (y >= renderer.height) { return false; }

  rect->x = CLAMP(x, 0, renderer.width - 1);
  rect->y = CLAMP(y, 0, renderer.height - 1);
  rect->w = CLAMP(rect->x + w, 0, renderer.width) - rect->x;
  rect->h = CLAMP(rect->y + h, 0, renderer.height) - rect->y;
  if (rect->w <= 0) { return false; }
  if (rect->h <= 0) { return false; }
  return true;
}

Color lerp_color(Color a, Color b, f32 t) {
  return (Color) {
    .r = lerp(a.r, b.r, t),
    .g = lerp(a.g, b.g, t),
    .b = lerp(a.b, b.b, t),
    .a = lerp(a.a, b.a, t)
  };
}

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.buffer = buffer;
  renderer.clear_buffer = clear_buffer;
  renderer.width = width;
  renderer.height = height;
  renderer.blend_mode = BLEND_NONE;
}

void renderer_set_blend_mode(Blend mode) {
  renderer.blend_mode = mode;
}

void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color) {
  Rect rect = {0};
  if (!renderer_normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry) {
    for (i32 rx = rect.x; rx < rect.x + rect.w; ++rx) {
      Color* target = renderer_get_pixel_addr(rx, ry);
      renderer_draw_pixel(target, color);
    }
  }
}

void render_fill_rect_gradient(i32 x, i32 y, i32 w, i32 h, Color color_start, Color color_end, v2 gradient_start, v2 gradient_end) {
  Rect rect = {0};
  if (!renderer_normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  w = rect.w;
  h = rect.h;
  v2 uv = V2(0, 0);
  for (i32 ry = rect.y, y = 0; ry < rect.y + rect.h; ++ry, ++y) {
    for (i32 rx = rect.x, x = 0; rx < rect.x + rect.w; ++rx, ++x) {
      Color* target = renderer_get_pixel_addr(rx, ry);
      uv = V2(x / (f32)w, y / (f32)h);
      f32 a = v2_dot(uv, gradient_start);
      f32 b = v2_dot(uv, gradient_end);
      if (a < b) {
        renderer_draw_pixel(target, lerp_color(color_start, color_end, a));
      }
      else {
        renderer_draw_pixel(target, lerp_color(color_start, color_end, b));
      }
    }
  }
}

void renderer_set_clear_color(Color color) {
  for (i32 i = 0; i < renderer.width * renderer.height; ++i) {
    renderer.clear_buffer[i] = color;
  } 
}

void render_clear(void) {
  memcpy(renderer.buffer, renderer.clear_buffer, sizeof(Color) * renderer.width * renderer.height);
}
