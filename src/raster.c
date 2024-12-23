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
} Game;

Game game = {
  .x = 0,
  .y = 0,
};

const u32 WINDOW_WIDTH = 800;
const u32 WINDOW_HEIGHT = 600;
u32 BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};
u32 CLEAR_BUFFER[WINDOW_WIDTH * WINDOW_HEIGHT] = {0};

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
  printf("x: %d, y: %d\n", x, y);
}

void input_event(i32 code) {

}

void update_and_render(double dt) {
  i32 fps = (i32)(1.0f / dt);
  render_clear();
  render_fill_rect(game.x, game.y, 32, 32, COLOR_RGBA(255, 0, 0, 255));
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
