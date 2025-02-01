// window_sdl.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#define ASPECT_RATIO (WINDOW_WIDTH / (f32)WINDOW_HEIGHT)
#define ASPECT_RATIO_INV (WINDOW_HEIGHT / (f32)WINDOW_WIDTH)

// https://www.freepascal-meets-sdl.net/sdl-2-0-scancode-lookup-table/
static const Key_event key_map[] = {
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_A,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_0,
  KEY_UNDEFINED, // return
  KEY_UNDEFINED, // escape
  KEY_UNDEFINED, // backspace
  KEY_UNDEFINED, // tab
  KEY_SPACE,

  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,
  KEY_UNDEFINED,

  KEY_RIGHT_ARROW,
  KEY_LEFT_ARROW,
  KEY_DOWN_ARROW,
  KEY_UP_ARROW,
};

const size_t KEY_MAP_SIZE = LENGTH(key_map);

struct {
  i32 width;
  i32 height;
  u32 raster_width;
  u32 raster_height;
  bool fullscreen;
  SDL_Texture* display_texture;
  SDL_Window* window;
  SDL_Renderer* renderer;
} window;

static void sdl_check_status(i32 status);
static void sdl_check_pointer(void* p);

void sdl_check_status(i32 status) {
  if (status < 0) {
    dprintf(STDERR_FILENO, "%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

void sdl_check_pointer(void* p) {
  if (!p) {
    dprintf(STDERR_FILENO, "%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

Result window_create() {
  sdl_check_status(SDL_Init(SDL_INIT_VIDEO));
  window.width = WINDOW_WIDTH;
  window.height = WINDOW_HEIGHT;
  window.raster_width = display_get_width();
  window.raster_height = display_get_height();
  window.fullscreen = false;
  window.window = SDL_CreateWindow(
    "",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    window.width,
    window.height,
    SDL_WINDOW_OPENGL | window.fullscreen * SDL_WINDOW_FULLSCREEN_DESKTOP
  );
  sdl_check_pointer(window.window);

  window.renderer = SDL_CreateRenderer(
    window.window,
    -1,
    SDL_RENDERER_ACCELERATED
  );
  sdl_check_pointer(window.renderer);

  window.display_texture = SDL_CreateTexture(
    window.renderer,
    SDL_PIXELFORMAT_ABGR8888,
    SDL_TEXTUREACCESS_STREAMING,
    window.raster_width,
    window.raster_height
  );
  sdl_check_pointer(window.display_texture);
#if 0
  struct SDL_RendererInfo info;
  SDL_GetRendererInfo(window.renderer, &info);
  printf("using renderer: %s\n", info.name);
#endif
  return Ok;
}

void window_set_title(char* title) {
  SDL_SetWindowTitle(window.window, title);
}

void window_render() {
  SDL_GetWindowSize(window.window, &window.width, &window.height);

  f32 w_aspect = window.width / (f32)window.raster_width;
  f32 h_aspect = window.height / (f32)window.raster_height;
  f32 aspect = w_aspect / h_aspect;
  (void)aspect; // unused
  f32 aspect_inv = MAX(h_aspect / w_aspect, 0);

  Color* color_buffer = display_get_addr();
  const SDL_Rect display_rect = {
    (1-aspect_inv) * window.width*0.5f, 0, window.width * aspect_inv, window.height
  };
  const SDL_Rect raster_rect = {
    0, 0, window.raster_width, window.raster_height
  };
  void* pixels = NULL;
  i32 pitch = 0;
  SDL_LockTexture(window.display_texture, &raster_rect, &pixels, &pitch);
  for (size_t y = 0; y < window.raster_height; ++y) {
    memcpy(
      (u8*)pixels + y * pitch,
      color_buffer + y * window.raster_width,
      window.raster_width * 4
    );
  }
  SDL_UnlockTexture(window.display_texture);
  SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, 255);
  SDL_RenderClear(window.renderer);
  SDL_RenderCopy(
    window.renderer,
    window.display_texture,
    &raster_rect,
    &display_rect
  );
  SDL_RenderPresent(window.renderer);
}

Result window_poll_events() {
  SDL_Event event = {0};
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: {
        return Error;
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          input_mouse_left_click(event.button.x, event.button.y);
        }
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          input_mouse_left_release(event.button.x, event.button.y);
        }
      }
      case SDL_MOUSEMOTION: {
        input_mouse_move(event.button.x, event.button.y);
        break;
      }
      case SDL_KEYDOWN: {
        i32 code = event.key.keysym.scancode;
        if (code >= 0 && code < KEY_MAP_SIZE) {
          input_key_down(key_map[code]);
        }
        break;
      }
      case SDL_KEYUP: {
        i32 code = event.key.keysym.scancode;
        if (code >= 0 && code < KEY_MAP_SIZE) {
          input_key_up(key_map[code]);
        }
        break;
      }
      default:
        break;
    }
  }
  return Ok;
}

size_t window_get_ticks() {
  return SDL_GetPerformanceCounter();
}

size_t window_get_freq() {
  return SDL_GetPerformanceFrequency();
}

void window_destroy() {
  SDL_DestroyTexture(window.display_texture);
  SDL_DestroyWindow(window.window);
  SDL_DestroyRenderer(window.renderer);
  SDL_Quit();
}

void window_toggle_fullscreen(void) {
  window.fullscreen = !window.fullscreen;
  if (window.fullscreen) {
    SDL_SetWindowFullscreen(window.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  }
  else {
    SDL_SetWindowFullscreen(window.window, 0);
  }

  SDL_GetWindowSize(window.window, &window.width, &window.height);
}
