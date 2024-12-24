// renderer.c

typedef struct Renderer {
  Color* buffer;
  Color* clear_buffer;
  u32 width;
  u32 height;
  Blend blend_mode;
  bool dither;
  i32 num_primitives;         // triangles drawn
  i32 num_primitives_culled;  // triangles culled
} Renderer;

static Renderer renderer;

static Color* get_pixel_addr(i32 x, i32 y);
static void draw_pixel(Color* pixel, Color color);
static bool normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect);
static bool triangle_bb(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Rect* rect);
static Color lerp_color(Color a, Color b, f32 t);
static bool bounds_check(Rect rect, i32 x, i32 y);
static bool fb_bounds_check(i32 x, i32 y);
static bool barycentric(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 x, i32 y, i32* u1, i32* u2, i32* det);

Color* get_pixel_addr(i32 x, i32 y) {
  return &renderer.buffer[y * renderer.width + x];
}

void draw_pixel(Color* pixel, Color color) {
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
bool normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect) {
  if (w <= 0) { return false; }
  if (h <= 0) { return false; }
  if (x >= renderer.width) { return false; }
  if (y >= renderer.height) { return false; }

  rect->x = CLAMP(x, 0, renderer.width);
  rect->y = CLAMP(y, 0, renderer.height);
  rect->w = CLAMP(rect->x + w, 0, renderer.width - 1) - rect->x;
  rect->h = CLAMP(rect->y + h, 0, renderer.height - 1) - rect->y;
  if (rect->w <= 0) { return false; }
  if (rect->h <= 0) { return false; }
  return true;
}

bool triangle_bb(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Rect* rect) {

  // triangle bounding box
  i32 min_x = ABS(i32, MIN3(x1, x2, x3));
  i32 min_y = ABS(i32, MIN3(y1, y2, y3));
  i32 max_x = ABS(i32, MAX3(x1, x2, x3));
  i32 max_y = ABS(i32, MAX3(y1, y2, y3));

  // clamp to framebuffer edges
  min_x = MAX(min_x, 0);
  min_y = MAX(min_y, 0);
  max_x = MIN(max_x, (i32)renderer.width - 1);
  max_y = MIN(max_y, (i32)renderer.height - 1);

  rect->x1 = min_x;
  rect->y1 = min_y;
  rect->x2 = max_x;
  rect->y2 = max_y;

  return ABS(i32, max_x - min_x) > 0 && ABS(i32, max_y - min_y) > 0;
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

// returns true if point (x, y) is inside the triangle
bool barycentric(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 x, i32 y, i32* u1, i32* u2, i32* det) {

  *det = (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
  *u1  = (y2 - y3) * (x - x3)  + (x3 - x2) * (y - y3);
  *u2  = (y3 - y1) * (x - x3)  + (x1 - x3) * (y - y3);

  i32 u3 = *det - *u1 - *u2;
  return (
    (SIGN(i32, *u1) == SIGN(i32, *det) || *u1 == 0) &&
    (SIGN(i32, *u2) == SIGN(i32, *det) || *u2 == 0) &&
    (SIGN(i32, u3)  == SIGN(i32, *det) ||  u3 == 0)
  );
}

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.buffer = buffer;
  renderer.clear_buffer = clear_buffer;
  renderer.width = width;
  renderer.height = height;
  renderer.blend_mode = BLEND_NONE;
  renderer.dither = true;
  renderer.num_primitives = 0;
  renderer.num_primitives_culled = 0;
}

void renderer_set_blend_mode(Blend mode) {
  renderer.blend_mode = mode;
}

void render_rect(i32 x, i32 y, i32 w, i32 h, Color color) {
  Rect rect = {0};
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  for (i32 rx = rect.x; rx <= rect.x + rect.w; ++rx) {
    Color* target = NULL;
    target = get_pixel_addr(rx, rect.y);
    draw_pixel(target, color);
    target = get_pixel_addr(rx, rect.y + rect.h);
    draw_pixel(target, color);
  }
  for (i32 ry = rect.y; ry <= rect.y + rect.h; ++ry) {
    Color* target = NULL;
    target = get_pixel_addr(rect.x, ry);
    draw_pixel(target, color);
    target = get_pixel_addr(rect.x + rect.w, ry);
    draw_pixel(target, color);
  }
}

void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color) {
  Rect rect = {0};
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry) {
    for (i32 rx = rect.x; rx < rect.x + rect.w; ++rx) {
      Color* target = get_pixel_addr(rx, ry);
      draw_pixel(target, color);
    }
  }
}

void render_fill_rect_gradient(i32 x, i32 y, i32 w, i32 h, Color color_start, Color color_end, v2 gradient_start, v2 gradient_end) {
  Rect rect = {0};
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  w = rect.w;
  h = rect.h;
  v2 uv = V2(0, 0);
  for (i32 ry = rect.y, y = 0; ry < rect.y + rect.h; ++ry, ++y) {
    for (i32 rx = rect.x, x = 0; rx < rect.x + rect.w; ++rx, ++x) {
      Color* target = get_pixel_addr(rx, ry);
      uv = V2(x / (f32)w - 1, y / (f32)h - 1);
      f32 a = -v2_dot(uv, gradient_start);
      f32 b = -v2_dot(uv, gradient_end);
      if (a < b) {
        draw_pixel(target, lerp_color(color_start, color_end, a));
      }
      else {
        draw_pixel(target, lerp_color(color_start, color_end, b));
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
      if (fb_bounds_check(y, x)) {
        Color* target = get_pixel_addr(y, x);
        draw_pixel(target, color);
      }
    }
    else {
      if (fb_bounds_check(x, y)) {
        Color* target = get_pixel_addr(x, y);
        draw_pixel(target, color);
      }
    }
  }
#else
  i32 dx = x2 - x1;
  i32 dy = y2 - y1;
  if (dx == dy && dx == 0) {
    if (fb_bounds_check(x1, y1)) {
      Color* target = get_pixel_addr(x1, y1);
      draw_pixel(target, color);
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
        Color* target = get_pixel_addr(x, y);
        draw_pixel(target, color);
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
        Color* target = get_pixel_addr(x, y);
        draw_pixel(target, color);
      }
    }
  }
#endif
}

void render_fill_triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color) {

  bool inside = false;
  Rect bb = {0};
  if (!triangle_bb(x1, y1, x2, y2, x3, y3, &bb)) {
    renderer.num_primitives_culled += 1;
    return;
  }

  for (i32 y = bb.y1; y < bb.y2; ++y) {
    for (i32 x = bb.x1; x < bb.x2; ++x) {
      i32 u1, u2, det;
      if (barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det)) {
        Color* target = get_pixel_addr(x, y);
        draw_pixel(target, color);
      }
    }
  }
  renderer.num_primitives += 1;
}

void renderer_set_clear_color(Color color) {
  for (i32 i = 0; i < renderer.width * renderer.height; ++i) {
    renderer.clear_buffer[i] = color;
  } 
}

void renderer_begin(void) {
  renderer.num_primitives = 0;
  renderer.num_primitives_culled = 0;
}

void render_post(void) {
  if (renderer.dither) {
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* color = get_pixel_addr(x, y);
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

i32 renderer_get_num_primitives(void) {
  return renderer.num_primitives;
}

i32 renderer_get_num_primitives_culled(void) {
  return renderer.num_primitives_culled;
}
