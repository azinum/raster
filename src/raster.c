// raster.c

#ifndef NO_OMP
  #include <omp.h>
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define USE_STB_SPRINTF
#define COMMON_IMPLEMENTATION
#include "common.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

#include "raster.h"

#include "maths.h"
#include "texture.h"
#include "font.h"
#include "config.h"
#include "camera.h"
#include "mesh.h"
#include "assets.h"
#include "light.h"
#include "renderer.h"
#include "window.h"

#include "maths.c"
#include "texture.c"
#include "font.c"
#include "camera.c"
#include "mesh.c"
#include "light.c"
#include "renderer.c"
#include "window.c"

typedef struct Game {
  Light light;
  size_t tick;
  f32 timer;
  f32 time_scale;
  bool paused;
  bool running;
  f32 dt_min;
  f32 dt_max;
} Game;

Game game = {
  .light = {0},
  .tick = 0,
  .timer = 0,
  .time_scale = 1.0f,
  .paused = false,
  .running = true,
  .dt_min = 10000,
  .dt_max = 0,
};

Color BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};
Color CLEAR_BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};

#ifdef NO_TIMER
extern i32 time();
#endif

const i32 EVENT_TYPE_DOWN = 0;
const i32 EVENT_TYPE_UP = 1;

void init(void) {
  input_init();
  random_init(time(0));
  renderer_init((Color*)display_get_addr(), (Color*)&CLEAR_BUFFER[0], display_get_width(), display_get_height());
  renderer_set_render_target(RENDER_TARGET_CLEAR);
  render_fill_rect_gradient(0, 0, display_get_width(), display_get_height(), COLOR_RGB(5, 5, 5), COLOR_RGB(0, 0, 0), V2(0, -1), V2(0, -1));
  renderer_set_render_target(RENDER_TARGET_COLOR);
  camera_init(V3(0, 1.8, -2));
  camera.rotation.pitch = 0;
  camera_update();
  game.light = light_create(V3(0, 2.5f, -4.5f), 2.0f, 1.5f);
  game.dt_min = 1;
  game.dt_max = 0;
}

i32 raster_main(i32 argc, char** argv) {
#ifndef TARGET_WASM
#define MAX_TITLE_LEN 256
  char title[MAX_TITLE_LEN] = {0};
  if (window_create() == Ok) {
    size_t prev = window_get_ticks();
    size_t current = prev;
    f32 dt = 0;
    while (window_poll_events() == Ok) {
      prev = current;
      current = window_get_ticks();
      dt = ((current - prev) * 1000 / (f32)window_get_freq()) * 0.001f;
      dt = dt * !(dt > DT_MAX);
      update_and_render(dt);
      window_render();
      snprintf(title, MAX_TITLE_LEN, "Raster | %g fps | %.4f dt", (1.0f / dt), dt);
      window_set_title(title);
      clear_input_events();
    }
  }
#endif
  return EXIT_SUCCESS;
}

void mouse_click(i32 x, i32 y) {

}

void input_event(i32 code, i32 type) {
  if (code < 0 || code >= MAX_KEY_EVENT) {
    return;
  }
  switch (type) {
    case EVENT_TYPE_DOWN: {
      input_key_down(code);
      break;
    }
    case EVENT_TYPE_UP: {
      input_key_up(code);
      break;
    }
    default:
      break;
  }
}

void update_and_render(f32 dt) {
  static size_t wait_ticks = 0;
  wait_ticks += 1;
  if (wait_ticks > 30) {
    if (dt < game.dt_min) {
      game.dt_min = dt;
    }
    if (dt > game.dt_max) {
      game.dt_max = dt;
    }
  }
  dt = CLAMP(dt, DT_MIN, DT_MAX);

  i32 fps = (i32)(1.0f / dt);
  if (input.key_pressed[KEY_SPACE]) {
    game.paused = !game.paused;
  }

  f32 speed = 4.0f * game.time_scale;
  f32 light_adjust_speed = 2.0f ;
  f32 rotation_speed = 150.0f * game.time_scale;
  if (input.key_pressed[KEY_R]) {
    game.timer = 0;
    init();
  }
  if (input.key_pressed[KEY_F]) {
    window_toggle_fullscreen();
  }
  if (input.key_pressed[KEY_T]) {
    renderer_toggle_texture_mapping();
  }
  if (input.key_down[KEY_W]) {
    camera.pos = V3_OP(
      camera.pos,
      V3(
        camera.forward.x * speed * dt,
        0,
        camera.forward.z * speed * dt
      ),
      +
    );
  }
  if (input.key_down[KEY_S]) {
    camera.pos = V3_OP(
      camera.pos,
      V3(
        camera.forward.x * -speed * dt,
        0,
        camera.forward.z * -speed * dt
      ),
      +
    );
  }
  if (input.key_down[KEY_A]) {
    camera.rotation.yaw += rotation_speed * dt;
  }
  if (input.key_down[KEY_D]) {
    camera.rotation.yaw -= rotation_speed * dt;
  }
  if (input.key_down[KEY_Q]) {
    camera.rotation.pitch += rotation_speed * dt;
  }
  if (input.key_down[KEY_E]) {
    camera.rotation.pitch -= rotation_speed * dt;
  }
  if (input.key_down[KEY_Z]) {
    camera.pos.y -= speed * dt;
  }
  if (input.key_down[KEY_X]) {
    camera.pos.y += speed * dt;
  }
  if (input.key_down[KEY_1]) {
    game.light.strength = CLAMP(game.light.strength - light_adjust_speed * dt, 0, 20);
  }
  if (input.key_down[KEY_2]) {
    game.light.strength = CLAMP(game.light.strength + light_adjust_speed * dt, 0, 20);
  }
  if (input.key_down[KEY_3]) {
    game.light.radius = CLAMP(game.light.radius - light_adjust_speed * dt, 0.01f, 100);
  }
  if (input.key_down[KEY_4]) {
    game.light.radius = CLAMP(game.light.radius + light_adjust_speed * dt, 0.01f, 100);
  }
  if (input.key_down[KEY_UP_ARROW]) {
    game.light.pos.z -= speed * dt;
  }
  if (input.key_down[KEY_DOWN_ARROW]) {
    game.light.pos.z += speed * dt;
  }
  if (input.key_down[KEY_LEFT_ARROW]) {
    game.light.pos.x += speed * dt;
  }
  if (input.key_down[KEY_RIGHT_ARROW]) {
    game.light.pos.x -= speed * dt;
  }
  if (input.key_down[KEY_J]) {
    game.light.pos.y += speed * dt;
  }
  if (input.key_down[KEY_K]) {
    game.light.pos.y -= speed * dt;
  }
  if (input.key_pressed[KEY_6]) {
    renderer_toggle_dither();
  }
  if (input.key_pressed[KEY_7]) {
    renderer_toggle_fog();
  }
  if (input.key_pressed[KEY_8]) {
    renderer_toggle_depth_test();
  }
  if (input.key_pressed[KEY_9]) {
    renderer_toggle_render_zbuffer();
  }
  if (input.key_pressed[KEY_0]) {
    renderer_toggle_render_normal_buffer();
  }
  if (input.key_pressed[KEY_N]) {
    game.time_scale = CLAMP(game.time_scale - 0.05f, 0, 1);
  }
  if (input.key_pressed[KEY_M]) {
    game.time_scale = CLAMP(game.time_scale + 0.05f, 0, 1);
  }

  camera_update();

  renderer_begin_frame(dt);
  renderer_clear();

  render_mesh(&room_floor, &t_tile_23, V3(0, 0, 0), V3(1, 1, 1), V3(0, 0, 0), game.light);
  render_mesh(&room, &t_brick_6, V3(0, 0, 0), V3(1, 1, 1), V3(0, 0, 0), game.light);
  {
    f32 size = 1;
    render_mesh(&cube, &t_brick_6, V3(0, 1.5f * sinf(game.timer * 0.8f), -6), V3(size, size, size), V3(game.timer * 42, 100 + game.timer * 30, 200 + game.timer * 40), game.light);
  }
  {
    f32 size = 1;
    render_mesh(&cube, &t_brick_6, V3(2, sinf(game.timer * 0.8f) - 1.2f, -6), V3(size, size, size), V3(0, 0, 0), game.light);
  }

  TIMER_START();
  renderer_draw();
  f32 time_to_render = TIMER_END();
  renderer_post_process();
  {
    static char text[256] = {0};
    static size_t length = 0;
    if ((game.tick % 4) == 0) {
      length = snprintf(text, sizeof(text), "%.d fps\nprimitives: %d\n%g ms", (i32)(1.0f / dt), renderer_get_num_primitives(), time_to_render * 1000);
    }
    render_text(text, length, 2, 2, 1, COLOR_RGB(255, 255, 255));
  }
  // render_axis(V3(0, 0, 0));
  // render_texture_3d(&t_sun_icon, game.light.pos, 24, 24, COLOR_RGB(255, 0, 255), COLOR_RGB(255, 255, 100));
  renderer_end_frame();
  if (!game.paused) {
    game.tick += 1;
    game.timer += dt * game.time_scale;
  }
}

u32 display_get_width(void) {
  return RASTER_WIDTH;
}

u32 display_get_height(void) {
  return RASTER_HEIGHT;
}

void* display_get_addr(void) {
  return &BUFFER[0];
}

void clear_input_events(void) {
  input_refresh();
}
