// renderer.c

// #define DRAW_BB
// #define NO_LIGHTING
// #define UNIFORM_LIGHTING_POSITION
// #define NO_TEXTURES
// #define NO_RENDER_COMMANDS
// #define NO_NORMAL_BUFFER

#define BB_COLOR COLOR_RGBA(255, 255, 255, 150)

#define MAX_RENDER_COMMANDS (1024*4)
#define MAX_RENDER_TEXTURES (8)

#ifndef NO_RENDER_COMMANDS
typedef enum Render_command_type {
  RENDER_CMD_DRAW_TRIANGLE,
  RENDER_CMD_SET_TEXTURE,
  RENDER_CMD_ENABLE_DEPTH_TEST,
  RENDER_CMD_DISABLE_DEPTH_TEST,
  RENDER_CMD_ENABLE_TEXTURE_MAPPING,
  RENDER_CMD_DISABLE_TEXTURE_MAPPING,
  RENDER_CMD_SET_LIGHT,

  MAX_RENDER_COMMAND_TYPE,
} Render_command_type;

typedef struct Render_command {
  Render_command_type type;
  union {
    // TODO: add id mapping to texture and lights
    struct {
      Triangle triangle;
      v3 world_normal;
      v3 world_position;
      Texture texture;
      Light light;
    } prim;
  };
} __attribute__((aligned(CACHELINESIZE))) Render_command;

#endif

typedef struct Renderer {
  Color* target;
  Color* color_buffer;
  Color* clear_buffer;
  f32* zbuffer_target;
  f32 zbuffer[RASTER_WIDTH * RASTER_HEIGHT];
  f32 clear_zbuffer[RASTER_WIDTH * RASTER_HEIGHT];
  Color normal_buffer[RASTER_WIDTH * RASTER_HEIGHT];
  Color clear_normal_buffer[RASTER_WIDTH * RASTER_HEIGHT];
  i32 width;
  i32 height;
  Blend blend_mode;
  bool dither;
  bool fog;
  bool edge_detection;
  bool render_zbuffer;
  bool render_normal_buffer;
  i32 num_primitives;         // triangles drawn
  i32 num_primitives_culled;  // triangles culled
  f32 dt;
  bool depth_test;
  bool texture_mapping;

#ifndef NO_RENDER_COMMANDS
  Render_command render_commands[MAX_RENDER_COMMANDS];
  size_t render_command_count;
  Texture textures[MAX_RENDER_TEXTURES];
  size_t render_texture_count;
#endif
} Renderer;

static Renderer renderer;

static Color* get_pixel_addr(i32 x, i32 y);
static Color* get_pixel_addr_from_buffer(Color* buffer, i32 x, i32 y);
static Color* get_pixel_addr_bounds_checked(Color* buffer, i32 x, i32 y);
static void draw_pixel(Color* pixel, Color color);
static f32* get_zbuffer_addr(i32 x, i32 y);
static f32 get_zbuffer_value(i32 x, i32 y);
static f32 get_zbuffer_value_bounds_checked(i32 x, i32 y, f32 out_of_bounds_value);
static bool normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect);
static bool triangle_bb(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Rect* rect);
static Color color_lerp(Color a, Color b, f32 t);
static bool bounds_check(Rect rect, i32 x, i32 y);
static bool fb_bounds_check(i32 x, i32 y);
static bool barycentric(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 x, i32 y, i32* u1, i32* u2, i32* det);
static bool v3_barycentric(v3 a, v3 b, v3 c, v3 p, f32* u1, f32* u2, f32* det);
static v2 v2_cartesian(v2 a, v2 b, v2 c, f32 w1, f32 w2, f32 w3);
static bool degenerate(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3);
static u8 trivial_reject(f32 x, f32 y, const f32 x_min, const f32 x_max, const f32 y_min, const f32 y_max);
static i32 clip_vertices(Vertex* input, Vertex* output, i32 count, v3 plane_pos, v3 plane_normal);

#ifndef NO_RENDER_COMMANDS
static void push_render_command(const Render_command* cmd);
static void push_render_command_simple(Render_command_type cmd_type);
static void process_render_commands(void);
#endif // NO_RENDER_COMMANDS

inline Color* get_pixel_addr(i32 x, i32 y) {
#ifndef DEBUG_OUT_OF_BOUNDS
  ASSERT(x >= 0 && x < renderer.width && y >= 0 && y < renderer.height);
#endif
  return &renderer.target[y * renderer.width + x];
}

inline Color* get_pixel_addr_from_buffer(Color* buffer, i32 x, i32 y) {
#ifndef DEBUG_OUT_OF_BOUNDS
  ASSERT(x >= 0 && x < renderer.width && y >= 0 && y < renderer.height);
#endif
  return &buffer[y * renderer.width + x];
}

Color* get_pixel_addr_bounds_checked(Color* buffer, i32 x, i32 y) {
  if (x >= 0 && x < renderer.width && y >= 0 && y < renderer.height) {
    return &buffer[y * renderer.width + x];
  }
  return NULL;
}

// TODO: implement
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

inline f32* get_zbuffer_addr(i32 x, i32 y) {
#ifndef DEBUG_OUT_OF_BOUNDS
  ASSERT(x >= 0 && x < renderer.width && y >= 0 && y < renderer.height);
#endif
  return &renderer.zbuffer_target[y * renderer.width + x];
}

inline f32 get_zbuffer_value(i32 x, i32 y) {
#ifndef DEBUG_OUT_OF_BOUNDS
  ASSERT(x >= 0 && x < renderer.width && y >= 0 && y < renderer.height);
#endif
  return renderer.zbuffer_target[y * renderer.width + x];
}

f32 get_zbuffer_value_bounds_checked(i32 x, i32 y, f32 out_of_bounds_value) {
  if (x >= 0 && x < renderer.width && y >= 0 && y < renderer.height) {
    return renderer.zbuffer_target[y * renderer.width + x];
  }
  return out_of_bounds_value;
}

// clamp rect to boundary, occlude if not visible
bool normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect) {
  if ((w <= 0) || (h <= 0) || (x >= renderer.width) || (y >= renderer.height)) { return false; }

  rect->x = CLAMP(x, 0, renderer.width - 1);
  rect->y = CLAMP(y, 0, renderer.height - 1);
  rect->w = CLAMP(x + w, 0, renderer.width - 1) - rect->x;
  rect->h = CLAMP(y + h, 0, renderer.height - 1) - rect->y;
  return (rect->w > 0 && rect->h > 0);
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
  max_x = MIN(max_x, (i32)renderer.width);
  max_y = MIN(max_y, (i32)renderer.height);

  rect->x1 = min_x;
  rect->y1 = min_y;
  rect->x2 = max_x;
  rect->y2 = max_y;

  return (max_x - min_x) > 0 && (max_y - min_y) > 0;
}

Color color_lerp(Color a, Color b, f32 t) {
  return (Color) {
    .r = f32_lerp(a.r, b.r, t),
    .g = f32_lerp(a.g, b.g, t),
    .b = f32_lerp(a.b, b.b, t),
    .a = f32_lerp(a.a, b.a, t)
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

bool v3_barycentric(v3 a, v3 b, v3 c, v3 p, f32* u1, f32* u2, f32* det) {
  v3 p1 = V3_OP(b, a, -);
  v3 p2 = V3_OP(c, a, -);
  v3 p3 = V3_OP(p, a, -);

  f32 d00 = v3_dot(p1, p1);
  f32 d01 = v3_dot(p1, p2);
  f32 d11 = v3_dot(p2, p2);
  f32 d20 = v3_dot(p3, p1);
  f32 d21 = v3_dot(p3, p2);

  f32 denom = d00 * d11 - d01 * d01;
  if (denom != 0.0f) {
    *u1 = (d11 * d20 - d01 * d21) / denom;
    *u2 = (d00 * d21 - d01 * d20) / denom;
  }
  *det = 1.0f - *u1 - *u2;
  return *u1 >= 0 && *u2 >= 0 && *det >= 0 && *u1 + *u2 <= 1;
}

inline v2 v2_cartesian(v2 a, v2 b, v2 c, f32 w1, f32 w2, f32 w3) {
  return V2(
    (a.x * w1) + (b.x * w2) + (c.x * w3),
    (a.y * w1) + (b.y * w2) + (c.y * w3)
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

i32 clip_vertices(Vertex* input, Vertex* output, i32 count, v3 plane_pos, v3 plane_normal) {
  i32 output_count = 0;

  v3 plane = plane_from_pos_and_normal(plane_pos, plane_normal);

  for (i32 i = 0; i < count; ++i) {
    Vertex a = input[i];
    Vertex b = input[(i + 1) % count];
    Vertex c;
    f32 t = 0;
    line_plane_intersection(plane_pos, plane_normal, a.p, b.p, &t);

    c.p  = v3_lerp(a.p, b.p, t);
    c.wp = v3_lerp(a.wp, b.wp, t);
    c.uv = v2_lerp(a.uv, b.uv, t);

    if (!point_behind_plane(b.p, plane)) { // b inside
      if (point_behind_plane(a.p, plane)) { // a outside
        output[output_count++] = c;
      }
      output[output_count++] = b;
    }
    else if (!point_behind_plane(a.p, plane)) { // a inside
      output[output_count++] = c;
    }
  }

  return output_count;
}

#ifndef NO_RENDER_COMMANDS
void push_render_command(const Render_command* cmd) {
  ASSERT(cmd);
  if (renderer.render_command_count < MAX_RENDER_COMMANDS) {
    renderer.render_commands[renderer.render_command_count++] = *cmd;
  }
}

void push_render_command_simple(Render_command_type cmd_type) {
  if (renderer.render_command_count < MAX_RENDER_COMMANDS) {
    Render_command cmd = (Render_command) {
      .type = cmd_type,
    };
    renderer.render_commands[renderer.render_command_count++] = cmd;
  }
}

void process_render_commands(void) {
  i32 i = 0;

  #pragma omp parallel for
  for (i = 0; i < renderer.render_command_count; ++i) {
    Render_command* cmd = &renderer.render_commands[i];
    switch (cmd->type) {
      case RENDER_CMD_DRAW_TRIANGLE: {
        Texture* texture = &cmd->prim.texture;
        Triangle* t = &cmd->prim.triangle;
        if (texture->data) {
          render_triangle_advanced(t->a, t->b, t->c, texture, cmd->prim.world_normal, cmd->prim.world_position, cmd->prim.light);
        }
        break;
      }
      case RENDER_CMD_SET_TEXTURE: {
        ASSERT(!"don't use this");
        // if (renderer.render_texture_count < MAX_RENDER_TEXTURES) {
        //   renderer.textures[renderer.render_texture_count++] = cmd->texture;
        // }
        // current_texture = cmd->texture;
        break;
      }
      case RENDER_CMD_ENABLE_DEPTH_TEST: {
        renderer.depth_test = true;
        break;
      }
      case RENDER_CMD_DISABLE_DEPTH_TEST: {
        renderer.depth_test = false;
        break;
      }
      case RENDER_CMD_ENABLE_TEXTURE_MAPPING: {
        renderer.texture_mapping = true;
        break;
      }
      case RENDER_CMD_DISABLE_TEXTURE_MAPPING: {
        renderer.texture_mapping = false;
        break;
      }
      case RENDER_CMD_SET_LIGHT: {
        // light = cmd->light;
        ASSERT(!"don't use this");
      }
      default:
        break;
    }
  }
}

#endif // NO_RENDER_COMMANDS

void renderer_init(Color* color_buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.color_buffer = color_buffer;
  renderer.clear_buffer = clear_buffer;
  renderer_set_render_target(RENDER_TARGET_COLOR);
  renderer.zbuffer_target = renderer.zbuffer;
  renderer.width = width;
  renderer.height = height;
  renderer.blend_mode = BLEND_NONE;
  renderer.dither = DITHERING;
  renderer.fog = FOG;
  renderer.edge_detection = EDGE_DETECTION;
  renderer.render_zbuffer = false;
  renderer.render_normal_buffer = false;
  renderer.num_primitives = 0;
  renderer.num_primitives_culled = 0;
  renderer.dt = 0;
  renderer.depth_test = true;
  renderer.texture_mapping = true;
#ifndef NO_RENDER_COMMANDS
  renderer.render_command_count = 0;
  renderer.render_texture_count = 0;
#endif

  for (i32 i = 0; i < width * height; ++i) {
    renderer.clear_zbuffer[i] = 1.0f;
  }
  memcpy(&renderer.zbuffer_target[0], &renderer.clear_zbuffer[0], sizeof(f32) * width * height);
  for (i32 i = 0; i < width * height; ++i) {
    renderer.clear_normal_buffer[i] = COLOR_RGB(0, 0, 0);
  }
  memcpy(&renderer.normal_buffer[0], &renderer.clear_normal_buffer[0], sizeof(Color) * width * height);
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
        draw_pixel(target, color_lerp(color_start, color_end, a));
      }
      else {
        draw_pixel(target, color_lerp(color_start, color_end, b));
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

void render_line_3d(v3 p1, v3 p2, Color color) {
  m4 model = translate(V3(0, 0, 0));
  m4 mvp = m4_multiply(projection, m4_multiply(view, model));
  v3 vt[2] = {
    m4_multiply_v3(mvp, p1),
    m4_multiply_v3(mvp, p2),
  };
  if (vt[0].w < EPS || vt[1].w < EPS) {
    return;
  }
  vt[0] = v3_div_scalar(vt[0], vt[0].w);
  vt[1] = v3_div_scalar(vt[1], vt[1].w);
  if (trivial_reject(vt[0].x, vt[0].y, -1, 1, -1, 1) != 0 && trivial_reject(vt[1].x, vt[1].y, -1, 1, -1, 1) != 0) {
    return;
  }
  vt[0] = project_to_screen(vt[0], renderer.width, renderer.height);
  vt[1] = project_to_screen(vt[1], renderer.width, renderer.height);
  render_line(vt[0].x, vt[0].y, vt[1].x, vt[1].y, color);
}

void render_fill_triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color) {
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

void render_texture_triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, f32 z1, f32 z2, f32 z3, v2 uv1, v2 uv2, v2 uv3, const Texture* const texture, f32 light_contrib) {
  Rect bb = {0};
  if (!triangle_bb(x1, y1, x2, y2, x3, y3, &bb)) {
    renderer.num_primitives_culled += 1;
    return;
  }

  Color texel = COLOR_RGB(255, 0, 255);
#ifdef NO_TEXTURES
  texel = COLOR_RGB(
    255 * light_contrib,
    255 * light_contrib,
    255 * light_contrib
  );
#endif

#ifdef DRAW_BB
  render_rect(bb.x, bb.y, bb.w - bb.x, bb.h - bb.y, BB_COLOR);
#endif
  for (i32 y = bb.y1; y < bb.y2; ++y) {
    i32 x = bb.x1;
    for (; x < bb.x2; ++x) {
      f32* zvalue = get_zbuffer_addr(x, y);
      i32 u1, u2, det = 0;
      if (barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det)) {
        Color* target = get_pixel_addr(x, y);
        f32 w1 = 0, w2 = 0, w3 = 0;
        if (det != 0) {
          w1 = u1 / (f32)det;
          w2 = u2 / (f32)det;
        }
        w3 = 1.0f - w1 - w2;
        if (renderer.depth_test) {
          f32 z = (z1 * w1) + (z2 * w2) + (z3 * w3); // barycentric to cartesian conversion
          if (z < *zvalue) {
            *zvalue = z;
          }
          else {
            continue;
          }
        }
#ifndef NO_TEXTURES
        if (!renderer.texture_mapping) {
          continue;
        }
        v2 uv = v2_cartesian(uv1, uv2, uv3, w1, w2, w3);
        i32 x_coord = (ABS(i32, texture->width * uv.x)) % texture->width;
        i32 y_coord = (ABS(i32, texture->height * uv.y)) % texture->height;
        texel = texture_get_pixel(texture, x_coord, y_coord);
        texel.r *= light_contrib;
        texel.g *= light_contrib;
        texel.b *= light_contrib;
#endif
        draw_pixel(target, texel);
      }
    }
  }
  renderer.num_primitives += 1;
}

void render_triangle_advanced(Vertex a, Vertex b, Vertex c, const Texture* texture, v3 world_normal, v3 world_position, Light light) {
  Rect bb = {0};
  if (!triangle_bb(a.p.x, a.p.y, b.p.x, b.p.y, c.p.x, c.p.y, &bb)) {
    renderer.num_primitives_culled += 1;
    return;
  }

  Color texel = COLOR_RGB(255, 0, 255);
#ifdef DRAW_BB
  render_rect(bb.x, bb.y, bb.w - bb.x, bb.h - bb.y, BB_COLOR);
#endif

  f32 light_contrib = 0;
#ifndef NO_LIGHTING
  f32 light_contribs[3] = {
    light_calculate_contribution(light, a.wp, world_normal),
    light_calculate_contribution(light, b.wp, world_normal),
    light_calculate_contribution(light, c.wp, world_normal)
  };
#else
  f32 light_contribs[3] = {
    1, 1, 1
  };
#endif

  for (i32 y = bb.y1; y < bb.y2; ++y) {
    i32 x = bb.x1;
    Color* target = get_pixel_addr(x, y);
    f32* zvalue = get_zbuffer_addr(x, y);
    for (; x < bb.x2; ++x, ++target, ++zvalue) {
      i32 u1, u2, det = 0;
      // TODO: use v3_barycentric here instead
      if (barycentric(a.p.x, a.p.y, b.p.x, b.p.y, c.p.x, c.p.y, x, y, &u1, &u2, &det)) {
        f32 w1 = 0, w2 = 0, w3 = 0;
        if (det != 0) {
          w1 = u1 / (f32)det;
          w2 = u2 / (f32)det;
        }
        w3 = 1.0f - w1 - w2;
        if (renderer.depth_test) {
          f32 z = (a.p.z * w1) + (b.p.z * w2) + (c.p.z * w3);
          if (z < *zvalue) {
            *zvalue = z;
#ifndef NO_NORMAL_BUFFER
            renderer.normal_buffer[y * renderer.width + x] = COLOR_RGB(
              UINT8_MAX * (1 + world_normal.x) * 0.5f,
              UINT8_MAX * (1 + world_normal.y) * 0.5f,
              UINT8_MAX * (1 + world_normal.z) * 0.5f
            );
#endif
          }
          else {
            continue;
          }
        }
        light_contrib = light_contribs[0] * w1 + light_contribs[1] * w2 + light_contribs[2] * w3;
        texel = COLOR_RGB(255, 255, 255);
#ifndef NO_TEXTURES
        if (renderer.texture_mapping) {
          v2 uv = v2_cartesian(a.uv, b.uv, c.uv, w1, w2, w3);
          i32 x_coord = ABS(i32, texture->width * uv.x);
          i32 y_coord = ABS(i32, texture->height * uv.y);
          texel = texture_get_pixel_wrapped(texture, x_coord, y_coord);
        }
#endif
        texel.r *= light_contrib;
        texel.g *= light_contrib;
        texel.b *= light_contrib;
        draw_pixel(target, texel);
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

void render_fill_circle_3d(v3 pos, f32 r, Color color) {
  m4 model = translate(pos);
  m4 mvp = m4_multiply(projection, m4_multiply(view, model));
  v3 vt = m4_multiply_v3(mvp, V3(0, 0, 0));
  vt = v3_div_scalar(vt, vt.w);
  if (trivial_reject(vt.x, vt.y, -1, 1, -1, 1) != 0) {
    return;
  }
  if (vt.w < CAMERA_ZNEAR) {
    return;
  }
  vt.x += 1.0f;
  vt.y += 1.0f;
  vt.x *= 0.5f * renderer.width;
  vt.y *= 0.5f * renderer.height;
  render_fill_circle(vt.x, vt.y, r, color);
}

void render_point_3d(v3 pos, Color color) {
  m4 model = translate(pos);
  m4 mvp = m4_multiply(projection, m4_multiply(view, model));

  v3 vt = m4_multiply_v3(mvp, V3(0, 0, 0));
  vt = v3_div_scalar(vt, vt.w);
  if (trivial_reject(vt.x, vt.y, -1, 1, -1, 1) != 0) {
    return;
  }
  if (vt.w < CAMERA_ZNEAR) {
    return;
  }
  vt.x += 1.0f;
  vt.y += 1.0f;
  vt.x *= 0.5f * renderer.width;
  vt.y *= 0.5f * renderer.height;
  Color* target = get_pixel_addr((i32)vt.x, (i32)vt.y);
  *target = color;
}

void render_texture(Texture* texture, i32 x, i32 y, i32 w, i32 h) {
  Rect rect;
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  i32 x_max = x + w;
  i32 y_max = y + h;
  i32 ty = 0;
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry, ++ty) {
    i32 tx = 0;
    i32 rx = rect.x;
    i32 ydelta = y_max - ry;
    Color* target = get_pixel_addr(rx, ry);
    for (; rx < rect.x + rect.w; ++rx, ++tx, ++target) {
      i32 xdelta = x_max - rx;
      v2 uv = V2(xdelta / (f32)w, ydelta / (f32)h);
      Color color = texture_get_pixel_wrapped(texture, uv.x * texture->width, uv.y * texture->height);
      draw_pixel(target, color);
    }
  }
}

void render_texture_with_mask(Texture* texture, i32 x, i32 y, i32 w, i32 h, Color mask) {
  Rect rect;
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  i32 x_max = x + w;
  i32 y_max = y + h;
  i32 ty = 0;
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry, ++ty) {
    i32 tx = 0;
    i32 rx = rect.x;
    i32 ydelta = y_max - ry;
    Color* target = get_pixel_addr(rx, ry);
    for (; rx < rect.x + rect.w; ++rx, ++tx, ++target) {
      i32 xdelta = x_max - rx;
      v2 uv = V2(xdelta / (f32)w, ydelta / (f32)h);
      Color color = texture_get_pixel_wrapped(texture, uv.x * texture->width, uv.y * texture->height);
      if ((color.value & mask.value) != color.value) {
        draw_pixel(target, color);
      }
    }
  }
}

void render_texture_with_mask_and_tint(Texture* texture, i32 x, i32 y, i32 w, i32 h, Color mask, Color tint) {
  Rect rect;
  if (!normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  i32 x_max = x + w;
  i32 y_max = y + h;
  i32 ty = 0;
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry, ++ty) {
    i32 tx = 0;
    i32 rx = rect.x;
    i32 ydelta = y_max - ry;
    Color* target = get_pixel_addr(rx, ry);
    for (; rx < rect.x + rect.w; ++rx, ++tx, ++target) {
      i32 xdelta = x_max - rx;
      v2 uv = V2(xdelta / (f32)w, ydelta / (f32)h);
      Color color = texture_get_pixel_wrapped(texture, uv.x * texture->width, uv.y * texture->height);
      if ((color.value & mask.value) != color.value) {
        f32 inv = 1.0f / UINT8_MAX;
        color.r = CLAMP((color.r * tint.r) * inv, 0, UINT8_MAX);
        color.g = CLAMP((color.g * tint.g) * inv, 0, UINT8_MAX);
        color.b = CLAMP((color.b * tint.b) * inv, 0, UINT8_MAX);
        draw_pixel(target, color);
      }
    }
  }
}

// classic billboard
void render_texture_3d(Texture* texture, v3 pos, i32 w, i32 h, Color mask, Color tint) {
  m4 model = translate(pos);
  m4 mvp = m4_multiply(projection, m4_multiply(view, model));
  v3 vt = m4_multiply_v3(mvp, V3(0, 0, 0));
  if (vt.w < EPS) {
    return;
  }
  vt = v3_div_scalar(vt, vt.w);
  if (trivial_reject(vt.x, vt.y, -1, 1, -1, 1) != 0) {
    return;
  }
  if (vt.w < CAMERA_ZNEAR || vt.w > CAMERA_ZFAR) {
    return;
  }
  vt.x += 1.0f;
  vt.y += 1.0f;
  vt.x *= 0.5f * renderer.width;
  vt.y *= 0.5f * renderer.height;
  // NOTE: origin at center, therefore:
  vt.x -= w * 0.5f;
  vt.y -= h * 0.5f;
  render_texture_with_mask_and_tint(texture, vt.x, vt.y, w, h, mask, tint);
}

void render_axis(v3 origin) {
  render_line_3d(origin, V3_OP(origin, V3(0, 1, 0), +), COLOR_RGB(0, 255, 0));
  render_line_3d(origin, V3_OP(origin, V3(1, 0, 0), +), COLOR_RGB(255, 0, 0));
  render_line_3d(origin, V3_OP(origin, V3(0, 0, 1), +), COLOR_RGB(0, 0, 255));
}

// TODO: bb and early cull
// TODO: bvh?
// TODO: view frustum culling/clipping in actual clip space, not in NDC
void render_mesh(Mesh* mesh, Texture* texture, v3 position, v3 size, v3 rotation, Light light) {
  m4 model = translate(position);

  model = m4_multiply(model, rotate(rotation.y, V3(0, 1, 0)));
  model = m4_multiply(model, rotate(rotation.z, V3(0, 0, 1)));
  model = m4_multiply(model, rotate(rotation.x, V3(1, 0, 0)));

  model = m4_multiply(model, scale(size));

  m4 vp = m4_multiply(projection, view); // TODO: calculate once per frame
  m4 mvp = m4_multiply(projection, m4_multiply(view, model));

  #define MAX_VERTEX_OUTPUT 9
  Vertex input[MAX_VERTEX_OUTPUT] = {0};
  Vertex output[MAX_VERTEX_OUTPUT] = {0};
  Vertex* clip_buffer[2] = { input, output };
#ifndef NO_RENDER_COMMANDS
  // {
  //   Render_command cmd = (Render_command) { .type = RENDER_CMD_SET_LIGHT, .light = light, };
  //   push_render_command(&cmd);
  // }
  // {
  //   Render_command cmd = (Render_command) { .type = RENDER_CMD_SET_TEXTURE, .texture = *texture, };
  //   push_render_command(&cmd);
  // }
#endif

  // proj * view * model * pos
  for (i32 i = 0; i < mesh->vertex_index_count; i += 3) {
    // original vertices
    const v3 v[3] = {
      mesh->vertex[mesh->vertex_index[i + 0]],
      mesh->vertex[mesh->vertex_index[i + 1]],
      mesh->vertex[mesh->vertex_index[i + 2]],
    };

    const v2 uv[3] = {
      mesh->uv[mesh->uv_index[i + 0]],
      mesh->uv[mesh->uv_index[i + 1]],
      mesh->uv[mesh->uv_index[i + 2]],
    };

    // vertex in world position
    const v3 vp[3] = {
      m4_multiply_v3(model, v[0]),
      m4_multiply_v3(model, v[1]),
      m4_multiply_v3(model, v[2]),
    };
    v3 pos = position;
#ifndef UNIFORM_LIGHTING_POSITION
    // center of the triangle
    pos = V3_OP(V3_OP(vp[0], vp[1], +), vp[2], +);
    pos = V3_OP1(pos, 1/3.0f, *);
#endif

    v3 wline1 = v3_sub(vp[1], vp[0]);
    v3 wline2 = v3_sub(vp[2], vp[0]);
    v3 world_normal = v3_normalize_fast(v3_cross(wline1, wline2));

    // backface culling
    if (v3_dot(world_normal, V3_OP(camera.pos, vp[0], -)) < 0.0f) {
      continue;
    }

    // transformed vertices
    v3 vt[3] = {
      m4_multiply_v3(mvp, v[0]),
      m4_multiply_v3(mvp, v[1]),
      m4_multiply_v3(mvp, v[2]),
    };

    if (vt[0].w < EPS || vt[1].w < EPS || vt[2].w < EPS) {
      continue;
    }

    v3 line1 = v3_sub(vt[1], vt[0]);
    v3 line2 = v3_sub(vt[2], vt[0]);
    v3 view_normal = v3_normalize_fast(v3_cross(line1, line2));

    // ndc
    vt[0] = v3_div_scalar(vt[0], vt[0].w);
    vt[1] = v3_div_scalar(vt[1], vt[1].w);
    vt[2] = v3_div_scalar(vt[2], vt[2].w);

    // view frustum clipping
    i32 clip_buffer_index = 0;
    i32 output_count = 3;

    // prepare input vertices
    for (i32 input_index = 0; input_index < 3; ++input_index) {
      Vertex* v = &input[input_index];
      v->wp = vp[input_index];
      v->p = vt[input_index];
      v->uv = uv[input_index];
    }
    for (i32 plane_index = 0; plane_index < 6; ++plane_index, ++clip_buffer_index) {
      Vertex* input = clip_buffer[clip_buffer_index % LENGTH(clip_buffer)];
      Vertex* output = clip_buffer[(clip_buffer_index + 1) % LENGTH(clip_buffer)];

      switch (plane_index) {
        case 0: { // near
          output_count = clip_vertices(input, output, output_count, V3(0, 0, 0), V3(0, 0, 1));
          break;
        }
        case 1: { // far
          output_count = clip_vertices(input, output, output_count, V3(0, 0, 1), V3(0, 0, -1));
          break;
        }
        case 2: { // left
          output_count = clip_vertices(input, output, output_count, V3(-1, 0, 0), V3(1, 0, 0));
          break;
        }
        case 3: { // right
          output_count = clip_vertices(input, output, output_count, V3(1, 0, 0), V3(-1, 0, 0));
          break;
        }
        case 4: { // top
          output_count = clip_vertices(input, output, output_count, V3(0, -1, 0), V3(0, 1, 0));
          break;
        }
        case 5: { // bottom
          output_count = clip_vertices(input, output, output_count, V3(0, 1, 0), V3(0, -1, 0));
          break;
        }
        default:
          break;
      }
    }
    if (output_count == 0) {
      continue;
    }
    ASSERT(output_count < MAX_VERTEX_OUTPUT);

    Vertex* clipped = clip_buffer[clip_buffer_index % LENGTH(clip_buffer)];
    for (i32 vertex_index = 0; vertex_index < output_count; ++vertex_index) {
      Vertex* v = &clipped[vertex_index];
      v->p = project_to_screen(v->p, renderer.width, renderer.height);
    }
    Vertex first = clipped[0];
    for (i32 vertex_index = 1; vertex_index + 1 < output_count; vertex_index += 1) {
      v3 a = first.p;
      v3 b = clipped[vertex_index].p;
      v3 c = clipped[vertex_index + 1].p;
      if (degenerate(a.x, a.y, b.x, b.y, c.x, c.y)) {
        continue;
      }
#ifndef NO_RENDER_COMMANDS
      Render_command cmd = (Render_command) {
        .type = RENDER_CMD_DRAW_TRIANGLE,
        .prim = {
          .triangle = (Triangle) {
            first, clipped[vertex_index], clipped[vertex_index + 1],
          },
          .texture = *texture,
          .light = light,
          .world_normal = world_normal,
          .world_position = pos,
        },
      };
      push_render_command(&cmd);
#else
      render_triangle_advanced(first, clipped[vertex_index], clipped[vertex_index + 1], texture, world_normal, pos, light);
#endif
    }

    if (RENDER_VERTICES) {
      for (i32 vertex = 0; vertex < output_count; ++vertex) {
        Vertex v = clipped[vertex];
        Color color = COLOR_RGB(0xfd, 0xd8, 0x35);
        i32 x = v.p.x;
        i32 y = v.p.y;
        render_fill_circle(x, y, 2, color);
      }
    }
  }
}

// TODO: bb
void render_text(const char* text, size_t length, i32 x, i32 y, f32 size, Color tint) {
  Font font = default_font;
  i32 x_spacing = 1 + font.width * size;
  i32 y_spacing = 1 + font.height * size;
  i32 x_offset = x;
  i32 y_offset = y;
  Color mask = COLOR_RGB(255, 0, 255);

  for (i32 i = 0; i < length; ++i) {
    char code = text[i];
    if (code == '\n') {
      x_offset = x;
      y_offset += y_spacing;
      continue;
    }
    if (code == ' ') {
      x_offset += font.width;
      continue;
    }
    for (i32 gy = 0; gy < font.height; ++gy) {
      for (i32 gx = 0; gx < font.width; ++gx) {
        Color color = font_get_color(&font, gx, gy, code);
        if (color.value != mask.value) {
          f32 inv = 1.0f / UINT8_MAX;
          color.r = CLAMP((color.r * tint.r) * inv, 0, UINT8_MAX);
          color.g = CLAMP((color.g * tint.g) * inv, 0, UINT8_MAX);
          color.b = CLAMP((color.b * tint.b) * inv, 0, UINT8_MAX);
          render_fill_rect(x_offset + gx * size, y_offset + gy * size, size, size, color);
        }
      }
    }
    x_offset += x_spacing;
  }
}

void renderer_set_clear_color(Color color) {
  for (i32 i = 0; i < renderer.width * renderer.height; ++i) {
    renderer.clear_buffer[i] = color;
  } 
}

void renderer_begin_frame(f32 dt) {
  renderer.num_primitives = 0;
  renderer.num_primitives_culled = 0;
#ifndef NO_RENDER_COMMANDS
  renderer.render_command_count = 0;
  renderer.render_texture_count = 0;
#endif
  renderer.dt = dt;
}

void renderer_draw(void) {
#ifndef NO_RENDER_COMMANDS
  process_render_commands();
#endif
}

void renderer_post_process(void) {
#ifndef NO_NORMAL_BUFFER
  if (renderer.edge_detection) {
    f32 normalization_factor = 1.0f / UINT8_MAX;
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* target = get_pixel_addr(x, y);
        Color* sample = get_pixel_addr_from_buffer(renderer.normal_buffer, x, y);
        f32 f = 0;
        f32 sample_count = 0;
        v3 sample_v = V3_OP1(V3(sample->r, sample->g, sample->b), normalization_factor, *);
        // (1+c)*0.5
        sample_v = V3_OP1(sample_v, 0.5f, -);
        sample_v = V3_OP1(sample_v, 2, *);
        for (i32 sy = -1; sy < 1; ++sy) {
          for (i32 sx = -1; sx < 1; ++sx) {
            Color* n = get_pixel_addr_bounds_checked(renderer.normal_buffer, x + sx, y + sy);
            if ((sx == 0 && sy == 0) || !n) {
              continue;
            }
            v3 n_v = V3_OP1(V3(n->r, n->g, n->b), normalization_factor, *);
            n_v = V3_OP1(n_v, 0.5f, -);
            n_v = V3_OP1(n_v, 2, *);
            f += v3_dot(n_v, sample_v);
            sample_count += 1;
          }
        }
        f *= 1.0f / sample_count;
        *target = color_lerp(*target, EDGE_DETECTION_COLOR, 1-f);
      }
    }
  }
#endif

  if (renderer.fog) {
    Color fog_color = FOG_COLOR; // COLOR_RGB(210, 210, 230);
    // f(x) = 1 / (ax * bx * cx);
    f32 falloff_a = 500;
    f32 falloff_b = 150;
    f32 falloff_c = 4;
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* color = get_pixel_addr(x, y);
        f32 z = 1 - get_zbuffer_value(x, y);
        f32 z_adjusted = CLAMP(1.0f / (1.0f + (falloff_a * z * falloff_b * z * falloff_c * z)), 0.0f, 1.0f);
        *color = color_lerp(*color, fog_color, z_adjusted);
      }
    }
  }
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

void renderer_end_frame(void) {
  if (renderer.render_zbuffer) {
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* color = get_pixel_addr(x, y);
        f32 z = *get_zbuffer_addr(x, y);
        u8 c = UINT8_MAX * (z * z * z * z);
        *color = COLOR_RGB(c, c, c);
      }
    }
  }
#ifndef NO_NORMAL_BUFFER
  else if (renderer.render_normal_buffer) {
    for (i32 y = 0; y < renderer.height; ++y) {
      for (i32 x = 0; x < renderer.width; ++x) {
        Color* color = get_pixel_addr(x, y);
        Color* normal = get_pixel_addr_from_buffer(renderer.normal_buffer, x, y);
        *color = *normal;
      }
    }
  }
#endif
}

void renderer_clear(void) {
  memcpy(renderer.color_buffer, renderer.clear_buffer, sizeof(Color) * renderer.width * renderer.height);
  memcpy(renderer.zbuffer_target, renderer.clear_zbuffer, sizeof(f32) * renderer.width * renderer.height);
#ifndef NO_NORMAL_BUFFER
  memcpy(&renderer.normal_buffer[0], &renderer.clear_normal_buffer[0], sizeof(Color) * renderer.width * renderer.height);
#endif
}

i32 renderer_get_num_primitives(void) {
  return renderer.num_primitives;
}

i32 renderer_get_num_primitives_culled(void) {
  return renderer.num_primitives_culled;
}

void renderer_toggle_fog(void) {
  renderer.fog = !renderer.fog;
}

void renderer_toggle_dither(void) {
  renderer.dither = !renderer.dither;
}

void renderer_toggle_depth_test(void) {
  renderer.depth_test = !renderer.depth_test;
}

void renderer_toggle_render_zbuffer(void) {
  renderer.render_zbuffer = !renderer.render_zbuffer;
  renderer.render_normal_buffer &= !renderer.render_zbuffer;
}

void renderer_toggle_render_normal_buffer(void) {
  renderer.render_normal_buffer = !renderer.render_normal_buffer;
  renderer.render_zbuffer &= !renderer.render_normal_buffer;
}

void renderer_toggle_texture_mapping(void) {
  renderer.texture_mapping = !renderer.texture_mapping;
}
