// renderer.c

typedef struct Renderer {
  Color* buffer;
  Color* clear_buffer;
  u32 width;
  u32 height;
  Blend blend_mode;
  bool dither;
} Renderer;

static Renderer renderer;

static Color* renderer_get_pixel_addr(i32 x, i32 y);
static void renderer_draw_pixel(Color* pixel, Color color);
static bool renderer_normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect);
static Color lerp_color(Color a, Color b, f32 t);
static bool bounds_check(Rect rect, i32 x, i32 y);
static bool fb_bounds_check(i32 x, i32 y);

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

bool bounds_check(Rect rect, i32 x, i32 y) {
  return (x >= rect.x && x < rect.w + rect.x) && (y >= rect.y && y < rect.h + rect.y);
}

bool fb_bounds_check(i32 x, i32 y) {
  return bounds_check(RECT(0, 0, renderer.width, renderer.height), x, y);
}

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.buffer = buffer;
  renderer.clear_buffer = clear_buffer;
  renderer.width = width;
  renderer.height = height;
  renderer.blend_mode = BLEND_NONE;
  renderer.dither = true;
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

// TODO: line normalization (clip to bounds, bb check)
void render_line(i32 x1, i32 y1, i32 x2, i32 y2, Color color) {
#if 1
  if ((x1 < 0 && x2 < 0) || (x1 >= renderer.width && x2 >= renderer.width)) { return; }
  if ((y1 < 0 && y2 < 0) || (y1 >= renderer.height && y2 >= renderer.height)) { return; }

  bool steep = false;
  if (ABS(i32, x1 - x2) < ABS(i32, y1 - y2)) {
    steep = true;
    SWAP(x1, y1);
    SWAP(x2, y2);
  }
  if (x1 > x2) {
    SWAP(x1, x2);
    SWAP(y1, y2);
  }

  for (f32 x = x1; x < x2; ++x) {
    f32 t = (x - x1) / (f32)(x2 - x1);
    f32 y = y1 * (1.0f - t) + (y2 * t);
    if (steep) {
      Color* target = renderer_get_pixel_addr(y, x);
      renderer_draw_pixel(target, color);
    }
    else {
      Color* target = renderer_get_pixel_addr(x, y);
      renderer_draw_pixel(target, color);
    }
  }
#else
  i32 dx = x2 - x1;
  i32 dy = y2 - y1;
  if (dx == dy && dx == 0) {
    if (fb_bounds_check(x1, y1)) {
      Color* target = renderer_get_pixel_addr(x1, y1);
      renderer_draw_pixel(target, color);
      return;
    }
  }

  if (ABS(i32, dx) > ABS(i32, dy)) {
    if (x1 > x2) {
      SWAP(x1, x2);
      SWAP(y1, y2);
    }
    for (i32 x = x1; x <= x2; ++x) {
      i32 y = dy * (x - x1) / dx + y1;
      if (fb_bounds_check(x, y)) {
        Color* target = renderer_get_pixel_addr(x, y);
        renderer_draw_pixel(target, color);
      }
    }
  }
  else {
    if (y1 > y2) {
      SWAP(x1, x2);
      SWAP(y1, y2);
    }
    for (i32 y = y1; y <= y2; ++y) {
      i32 x = dx * (y - y1) / dy + x1;
      if (fb_bounds_check(x, y)) {
        Color* target = renderer_get_pixel_addr(x, y);
        renderer_draw_pixel(target, color);
      }
    }
  }
#endif
}

void renderer_set_clear_color(Color color) {
  for (i32 i = 0; i < renderer.width * renderer.height; ++i) {
    renderer.clear_buffer[i] = color;
  } 
}

void render_post(void) {
  if (renderer.dither) {
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* color = renderer_get_pixel_addr(x, y);
        u8 d = (x % 2) * (y % 2);
        color->r -= (color->r * 0.1f) * d;
        color->g -= (color->g * 0.1f) * d;
        color->b -= (color->b * 0.1f) * d;
      }
    }
  }
}

void render_clear(void) {
  memcpy(renderer.buffer, renderer.clear_buffer, sizeof(Color) * renderer.width * renderer.height);
}
