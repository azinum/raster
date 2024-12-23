// renderer.c

typedef struct Renderer {
  Color* buffer;
  Color* clear_buffer;
  u32 width;
  u32 height;
} Renderer;

static Renderer renderer;

static Color* renderer_get_pixel_addr(i32 x, i32 y);
static bool renderer_normalize_rect(i32 x, i32 y, i32 w, i32 h, Rect* rect);

Color* renderer_get_pixel_addr(i32 x, i32 y) {
  return &renderer.buffer[y * renderer.width + x];
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

void renderer_init(Color* buffer, Color* clear_buffer, u32 width, u32 height) {
  renderer.buffer = buffer;
  renderer.clear_buffer = clear_buffer;
  renderer.width = width;
  renderer.height = height;
}

void render_fill_rect(i32 x, i32 y, i32 w, i32 h, Color color) {
  Rect rect = {0};
  if (!renderer_normalize_rect(x, y, w, h, &rect)) {
    return;
  }
  for (i32 ry = rect.y; ry < rect.y + rect.h; ++ry) {
    for (i32 rx = rect.x; rx < rect.x + rect.w; ++rx) {
      Color* target = renderer_get_pixel_addr(rx, ry);
      *target = color;
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
