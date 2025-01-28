// window.c

Input input = {0};

void input_init() {
  memset(&input, 0, sizeof(Input));
  input.mouse_click = false;
  input.mouse_down = false;
}

void input_refresh() {
  memset(&input.key_pressed, 0, sizeof(input.key_pressed));
  input.mouse_click = false;
}

void input_key_down(Key_event code) {
  if (!input.key_down[code]) {
    input.key_down[code] = true;
    input.key_pressed[code] = true;
  }
}

void input_key_up(Key_event code) {
  input.key_down[code] = false;
}

void input_mouse_left_click(i32 x, i32 y) {
  input.mouse_click = true;
  input.mouse_down = true;
  input.mouse_x = x;
  input.mouse_y = y;
}

void input_mouse_left_release(i32 x, i32 y) {
  input.mouse_down = false;
  input.mouse_x = x;
  input.mouse_y = y;
}

void input_mouse_move(i32 x, i32 y) {
  input.mouse_x = x;
  input.mouse_y = y;
}

#ifndef TARGET_WASM
  #include "window_sdl.c"
#endif
