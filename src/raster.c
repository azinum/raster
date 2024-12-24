// raster.c

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define USE_STB_SPRINTF
#define COMMON_IMPLEMENTATION
#include "common.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

#include "maths.h"
#include "renderer.h"

#include "maths.c"
#include "renderer.c"

typedef struct Game {
  i32 x;
  i32 y;
  size_t tick;
  f32 timer;
  bool paused;
} Game;

Game game = {
  .x = 0,
  .y = 0,
  .tick = 0,
  .timer = 0,
  .paused = false,
};

const u32 WINDOW_WIDTH = 800 / 2;
const u32 WINDOW_HEIGHT = 600 / 2;
u32 BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};
u32 CLEAR_BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};

static f32 x_offset = 0;
static f32 y_offset = 0;

const i32 KEY_UNDEFINED = 0;
const i32 KEY_SPACE = 1;
const i32 KEY_LEFT_ARROW = 2;
const i32 KEY_UP_ARROW = 3;
const i32 KEY_RIGHT_ARROW = 4;
const i32 KEY_DOWN_ARROW = 5;
const i32 KEY_0 = 6;
const i32 KEY_1 = 7;
const i32 KEY_2 = 8;
const i32 KEY_3 = 9;
const i32 KEY_4 = 10;
const i32 KEY_5 = 11;
const i32 KEY_6 = 12;
const i32 KEY_7 = 13;
const i32 KEY_8 = 14;
const i32 KEY_9 = 15;
const i32 KEY_A = 16;
const i32 KEY_B = 17;
const i32 KEY_C = 18;
const i32 KEY_D = 19;
const i32 KEY_E = 20;
const i32 KEY_F = 21;
const i32 KEY_G = 22;
const i32 KEY_H = 23;
const i32 KEY_I = 24;
const i32 KEY_J = 25;
const i32 KEY_K = 26;
const i32 KEY_L = 27;
const i32 KEY_M = 28;
const i32 KEY_N = 29;
const i32 KEY_O = 30;
const i32 KEY_P = 31;
const i32 KEY_Q = 32;
const i32 KEY_R = 33;
const i32 KEY_S = 34;
const i32 KEY_T = 35;
const i32 KEY_U = 36;
const i32 KEY_V = 37;
const i32 KEY_W = 38;
const i32 KEY_X = 39;
const i32 KEY_Y = 40;
const i32 KEY_Z = 41;

void init(void);
void mouse_click(i32 x, i32 y);
void input_event(i32 code);
void update_and_render(double dt);
u32 display_get_width(void);
u32 display_get_height(void);
u32* display_get_addr(void);

void init(void) {
  random_init(1234);
  renderer_init((Color*)display_get_addr(), (Color*)&CLEAR_BUFFER[0], display_get_width(), display_get_height());
  renderer_set_clear_color(COLOR_RGBA(40, 40, 40, 255));
}

void mouse_click(i32 x, i32 y) {
  game.x = x;
  game.y = y;
}

void input_event(i32 code) {
  switch (code) {
    case KEY_R: {
      game.timer = 0;
      x_offset = random_f32() * 1000;
      y_offset = random_f32() * 1000;
      break;
    }
    case KEY_SPACE: {
      game.paused = !game.paused;
      break;
    }
    default:
      break;
  }
}

void update_and_render(double dt) {
  i32 fps = (i32)(1.0f / dt);
  if (game.paused) {
    return;
  }

  renderer_begin();
  render_clear();
  render_fill_rect_gradient(
    game.x,
    game.y,
    32, 128,
    COLOR_RGBA(255, 0, 0, 255),
    COLOR_RGBA(0, 0, 0, 0),
    V2(0.0f, -1.0f),
    V2(0.0f, -1.0f)
  );
  render_rect(
    game.x,
    game.y,
    32, 128,
    COLOR_RGB(255, 255, 255)
  );
  render_fill_rect_gradient(
    100,
    50,
    64, 64,
    COLOR_RGBA(255, 0, 0, 255),
    COLOR_RGBA(0, 0, 0, 0),
    V2(0.0f, 1.0f),
    V2(0, 1.0f)
  );
  render_fill_rect_gradient(
    180,
    50,
    18, 24,
    COLOR_RGBA(255, 0, 0, 255),
    COLOR_RGBA(0, 0, 255, 255),
    V2(0.5f, 0.5f),
    V2(0.5f, 0.5f)
  );
  {
    i32 x = 40;
    i32 y = 100;
    i32 len = 20;
    render_line(x, y, x + len * cosf(game.timer), y + len * sinf(game.timer), COLOR_RGB(255, 20, 140));
    len += 2;
    y += len + 2;
    render_line(x + len * cosf(game.timer + 100), y + len * sinf(game.timer + 100), x, y, COLOR_RGB(255, 20, 140));
  }

  {
    i32 x = 200;
    i32 y = 100;
    render_fill_triangle(x, y, x + 40 + 200 * sinf(game.timer * 0.25f), y - 30, x + 20, y + 80, COLOR_RGB(0, 255, 255));
  }
  render_post();
  game.tick += 1;
  game.timer += dt;
}

u32 display_get_width(void) {
  return WINDOW_WIDTH;
}

u32 display_get_height(void) {
  return WINDOW_HEIGHT;
}

u32* display_get_addr(void) {
  return &BUFFER[0];
}
