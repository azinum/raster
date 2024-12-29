// raster.c

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define USE_STB_SPRINTF
#define COMMON_IMPLEMENTATION
#include "common.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

#include "maths.h"
#include "config.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "assets.h"
#include "light.h"
#include "renderer.h"

#include "maths.c"
#include "camera.c"
#include "mesh.c"
#include "texture.c"
#include "light.c"
#include "renderer.c"

typedef struct Game {
  v3 object;
  Light light;
  size_t tick;
  f32 timer;
  bool paused;
} Game;

#define OBJECT_INIT_POS V3(0, 0, -6)

Game game = {
  .object = OBJECT_INIT_POS,
  .light = {0},
  .tick = 0,
  .timer = 0,
  .paused = false,
};

Color BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};
Color CLEAR_BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};
f32 ZBUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};

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
void* display_get_addr(void);

void init(void) {
  random_init(1234);
  renderer_init((Color*)display_get_addr(), (Color*)&CLEAR_BUFFER[0], display_get_width(), display_get_height());
  renderer_set_render_target(RENDER_TARGET_CLEAR);
  render_fill_rect_gradient(0, 0, display_get_width(), display_get_height(), COLOR_RGB(30, 40, 65), COLOR_RGB(0, 0, 0), V2(0, -1), V2(0, -1));
  renderer_set_render_target(RENDER_TARGET_COLOR);
  camera_init(V3(0, -2, 0));
  camera.rotation.pitch = -30;
  camera_update();
  game.light = light_create(V3(0, 0, -4.5), 1.0f, 4.25f);
}

void mouse_click(i32 x, i32 y) {
  game.object = V3(0, 0, 0);
}

void input_event(i32 code) {
  f32 speed = 0.05f;
  switch (code) {
    case KEY_R: {
      game.timer = 0;
      game.object = OBJECT_INIT_POS;
      x_offset = random_f32() * 1000;
      y_offset = random_f32() * 1000;
      break;
    }
    case KEY_SPACE: {
      game.paused = !game.paused;
      break;
    }
    // forward, backward
    case KEY_W: {
      game.object.z -= speed;
      break;
    }
    case KEY_S: {
      game.object.z += speed;
      break;
    }
    // left, right
    case KEY_A: {
      game.object.x -= speed;
      break;
    }
    case KEY_D: {
      game.object.x += speed;
      break;
    }
    // up, down
    case KEY_Z: {
      game.object.y += speed;
      break;
    }
    case KEY_X: {
      game.object.y -= speed;
      break;
    }
    case KEY_1: {
      game.light.strength -= 0.1f;
      break;
    }
    case KEY_2: {
      game.light.strength += 0.1f;
      break;
    }
    case KEY_3: {
      game.light.radius -= 0.1f;
      break;
    }
    case KEY_4: {
      game.light.radius += 0.1f;
      break;
    }
    case KEY_UP_ARROW: {
      game.light.pos.z -= 0.5f;
      break;
    }
    case KEY_DOWN_ARROW: {
      game.light.pos.z += 0.5f;
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
#if 0
  {
    i32 x = game.x;
    i32 y = game.y;
    render_fill_triangle(x, y, x + 40, y - 30, x + 20, y + 100, COLOR_RGB(130, 100, 255));
  }
#endif

  for (i32 x = -7; x < 9; ++x) {
    for (i32 z = -12; z < 0; ++z) {
      render_mesh(&plane, V3(x, 1, z), V3(1, 1, 1), V3(0, 0, 0), game.light);
    }
  }
  {
    f32 size = 1; // 2 + sinf(game.timer + 225);
    render_mesh(&cube, game.object, V3(size, size, size), V3(game.timer * 42, 100 + game.timer * 30, 200 + game.timer * 40), game.light);
  }

#if 0
  f32 size = 1; //2 + sinf(game.timer + 225);
  // size *= 0.2f;
  for (i32 y = -3; y < 4; ++y) {
    for (i32 x = -5; x < 6; ++x) {
      for (i32 z = -8; z < 2; ++z) {
        render_mesh(&cube, V3(x * 0.5f, y * 0.5f, 0.5f * z - 6), V3(size, size, size), V3(game.timer * 42, 100 + game.timer * 30, 200 + game.timer * 40));
      }
    }
  }
#endif
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

void* display_get_addr(void) {
  return &BUFFER[0];
}
