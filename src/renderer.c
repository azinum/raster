// renderer.c

// #define DRAW_BB
#define BB_COLOR COLOR_RGBA(255, 255, 255, 150)
#define DEBUG_OUT_OF_BOUNDS

typedef struct Renderer {
  Color* target;
  Color* color_buffer;
  Color* clear_buffer;
  i32 width;
  i32 height;
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
static bool degenerate(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3);
static u8 trivial_reject(f32 x, f32 y, const f32 x_min, const f32 x_max, const f32 y_min, const f32 y_max);

inline Color* get_pixel_addr(i32 x, i32 y) {

#ifndef DEBUG_OUT_OF_BOUNDS
  ASSERT(x >= 0 && x < renderer.width && y >= 0 && y < renderer.height);
#endif
  return &renderer.target[y * renderer.width + x];
}

inline void draw_pixel(Color* pixel, Color color) {
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

  rect->x = CLAMP(x, 0, renderer.width - 1);
  rect->y = CLAMP(y, 0, renderer.height - 1);
  rect->w = CLAMP(x + w, 0, renderer.width - 1) - rect->x;
  rect->h = CLAMP(y + h, 0, renderer.height - 1) - rect->y;
  if (rect->w <= 0) { return false; }
  if (rect->h <= 0) { return false; }
  return true;
}

bool triangle_bb(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Rect* rect) {

  // triangle bounding box
  i32 min_x = MAX(MIN3(x1, x2, x3), 0);
  i32 min_y = MAX(MIN3(y1, y2, y3), 0);
  i32 max_x = MAX(MAX3(x1, x2, x3), 0);
  i32 max_y = MAX(MAX3(y1, y2, y3), 0);

  // clamp to framebuffer edges
  min_x = MAX(min_x, 0);
  min_y = MAX(min_y, 0);
  max_x = MIN(max_x, (i32)renderer.width - 1);
  max_y = MIN(max_y, (i32)renderer.height - 1);

  rect->x1 = min_x;
  rect->y1 = min_y;
  rect->x2 = max_x;
  rect->y2 = max_y;

  return (max_x - min_x) > 0 && (max_y - min_y) > 0;
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

inline bool degenerate(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3) {
  return (x1 == x2 && y1 == y2) || (x2 == x3 && y2 == y3);
}

// Cohen-Sutherland outcode for trivial reject/accept
inline u8 trivial_reject(f32 x, f32 y, const f32 x_min, const f32 x_max, const f32 y_min, const f32 y_max) {
  return
    (y > y_max) |
    (y < y_min) << 1 |
    (x > x_max) << 2 |
    (x < x_min) << 4;
}

void renderer_init(Color* color_buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.color_buffer = color_buffer;
  renderer.clear_buffer = clear_buffer;
  renderer_set_render_target(RENDER_TARGET_COLOR);
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

void renderer_set_render_target(Render_target render_target) {
  switch (render_target) {
    case RENDER_TARGET_COLOR: {
      renderer.target = renderer.color_buffer;
      break;
    }
    case RENDER_TARGET_CLEAR: {
      renderer.target = renderer.clear_buffer;
      break;
    }
    default:
      break;
  }
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
  if ((x1 < 0 && x2 < 0) || (x1 >= renderer.width && x2 >= renderer.width)) { return; }
  if ((y1 < 0 && y2 < 0) || (y1 >= renderer.height && y2 >= renderer.height)) { return; }

  bool steep = false;
  if (ABS(i32, x1 - x2) < ABS(i32, y1 - y2)) {
    steep = true;
    SWAP(i32, x1, y1);
    SWAP(i32, x2, y2);
  }
  if (x1 > x2) {
    SWAP(i32, x1, x2);
    SWAP(i32, y1, y2);
  }

  i32 ox1 = x1;
  i32 oy1 = y1;
  i32 ox2 = x2;
  i32 oy2 = y2;

  x1 = CLAMP(x1, 0, renderer.width - 1);
  y1 = CLAMP(y1, 0, renderer.height - 1);
  x2 = CLAMP(x2, 0, renderer.width - 1);
  y2 = CLAMP(y2, 0, renderer.height - 1);

  i32 dx = ox2 - ox1;
  i32 dy = oy2 - oy1;
  if (dx == dy && dx == 0) {
    if (fb_bounds_check(x1, y1)) {
      Color* target = get_pixel_addr(x1, y1);
      draw_pixel(target, color);
      return;
    }
  }

  for (f32 x = x1; x < x2; ++x) {
    f32 t = (x - ox1) / (ox2 - ox1);
    f32 y = oy1 * (1.0f - t) + (oy2 * t);
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
}

void render_fill_triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color) {

  bool inside = false;
  Rect bb = {0};
  if (!triangle_bb(x1, y1, x2, y2, x3, y3, &bb)) {
    renderer.num_primitives_culled += 1;
    return;
  }

#ifdef DRAW_BB
  render_rect(bb.x, bb.y, bb.w - bb.x, bb.h - bb.y, BB_COLOR);
#endif
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

void render_fill_circle(i32 px, i32 py, i32 r, Color color) {
  Rect rect = {0};
  if (!normalize_rect(px - r, py - r, 2*r, 2*r, &rect)) {
    return;
  }
#ifdef DRAW_BB
  render_rect(rect.x, rect.y, rect.w, rect.h, BB_COLOR);
#endif
  for (i32 y = rect.y; y <= rect.y + rect.h; ++y) {
    i32 x = rect.x;
    Color* target = get_pixel_addr(x, y);
    for (; x <= rect.x + rect.w; ++x, ++target) {
      i32 dx = (x * 2 - px * 2);
      i32 dy = (y * 2 - py * 2);
      if (dx * dx + dy * dy <= r * r * 2 * 2) {
        draw_pixel(target, color);
      }
    }
  }
}

// -z forward, +z back
// -x left, +x right
// +y up, -y down
void render_mesh(Mesh* mesh, v3 position, v3 size, v3 rotation) {
  const f32 z_near = 0.2f;
  const f32 z_far = 10.0;
  m4 proj = perspective(80, renderer.width / (f32)renderer.height, z_near, z_far);
  m4 model = translate(position);

  model = m4_multiply(model, rotate(rotation.y, V3(0, 1, 0)));
  model = m4_multiply(model, rotate(rotation.z, V3(0, 0, 1)));
  model = m4_multiply(model, rotate(rotation.x, V3(1, 0, 0)));

  model = m4_multiply(model, scale(size));

  static const Color palette[] = {
    COLOR_RGB(230, 100, 100),
    COLOR_RGB(100, 230, 100),
    COLOR_RGB(100, 100, 230),
    COLOR_RGB(230, 100, 230),
    COLOR_RGB(100, 230, 230),
    COLOR_RGB(230, 90, 230),
    COLOR_RGB(60, 150, 230),
  };
  random_init(1234);
  i32 color_index = 0;

  m4 mv = m4_multiply(proj, view);
  m4 mvp = m4_multiply(proj, m4_multiply(view, model));

  // proj * view * model * pos
  for (i32 i = 0; i < mesh->vertex_index_count; i += 3) {
    Color color = palette[color_index % LENGTH(palette)];
    color_index += 1;
    // original vertices
    v3 v[3] = {
      mesh->vertex[mesh->vertex_index[i + 0]],
      mesh->vertex[mesh->vertex_index[i + 1]],
      mesh->vertex[mesh->vertex_index[i + 2]],
    };

    // transformed vertices
    v3 vt[3] = {
      m4_multiply_v3(mvp, v[0]),
      m4_multiply_v3(mvp, v[1]),
      m4_multiply_v3(mvp, v[2]),
    };

    vt[0] = v3_div_scalar(vt[0], vt[0].w);
    vt[1] = v3_div_scalar(vt[1], vt[1].w);
    vt[2] = v3_div_scalar(vt[2], vt[2].w);

    float depth_avg = (vt[0].w + vt[1].w + vt[2].w) / 3.0f;

    v3 line1 = v3_sub(vt[1], vt[0]);
    v3 line2 = v3_sub(vt[2], vt[0]);
    v3 normal = v3_normalize(v3_cross(line1, line2));

    // backface culling
    if (v3_dot(normal, v3_sub(camera.pos, vt[0])) > 0.0f) {
      continue;
    }

    if (depth_avg <= z_near || depth_avg >= z_far) {
      continue;
    }

    // viewspace-transformed vertices used for clipping
    v3 vt_copy[3] = {
      vt[0],
      vt[1],
      vt[2],
    };

    if (trivial_reject(vt[0].x, vt[0].y, -1, 1, -1, 1) != 0 && trivial_reject(vt[1].x, vt[1].y, -1, 1, -1, 1) != 0 && trivial_reject(vt[2].x, vt[2].y, -1, 1, -1, 1) != 0) {
      continue;
    }

    // project to screen
    vt[0].x += 1.0f;
    vt[1].x += 1.0f;
    vt[2].x += 1.0f;
    vt[0].y += 1.0f;
    vt[1].y += 1.0f;
    vt[2].y += 1.0f;
    vt[0].x *= 0.5f * renderer.width;
    vt[1].x *= 0.5f * renderer.width;
    vt[2].x *= 0.5f * renderer.width;
    vt[0].y *= 0.5f * renderer.height;
    vt[1].y *= 0.5f * renderer.height;
    vt[2].y *= 0.5f * renderer.height;

    if (degenerate(vt[0].x, vt[0].y, vt[1].x, vt[1].y, vt[2].x, vt[2].y)) {
      continue;
    }

    // TODO(lucas): clip against view frustum

    render_fill_triangle(vt[0].x, vt[0].y, vt[1].x, vt[1].y, vt[2].x, vt[2].y, color);
    // render_line(vt[0].x, vt[0].y, vt[1].x, vt[1].y, COLOR_RGB(255, 255, 255));
    // render_line(vt[1].x, vt[1].y, vt[2].x, vt[2].y, COLOR_RGB(255, 255, 255));
    // render_line(vt[2].x, vt[2].y, vt[1].x, vt[1].y, COLOR_RGB(255, 255, 255));
  }
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
  memcpy(renderer.color_buffer, renderer.clear_buffer, sizeof(Color) * renderer.width * renderer.height);
}

i32 renderer_get_num_primitives(void) {
  return renderer.num_primitives;
}

i32 renderer_get_num_primitives_culled(void) {
  return renderer.num_primitives_culled;
}
