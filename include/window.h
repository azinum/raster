// window.h

#ifndef _WINDOW_H
#define _WINDOW_H

typedef enum {
  KEY_UNDEFINED,
  KEY_SPACE,
  KEY_LEFT_ARROW,
  KEY_UP_ARROW,
  KEY_RIGHT_ARROW,
  KEY_DOWN_ARROW,
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
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
  MAX_KEY_EVENT,
} Key_event;

typedef struct Input {
  bool key_down[MAX_KEY_EVENT];
  bool key_pressed[MAX_KEY_EVENT];
  bool mouse_click;
  bool mouse_down;
  i32 mouse_x;
  i32 mouse_y;
} Input;

Input input;

void input_init();
void input_refresh();
void input_key_down(Key_event code);
void input_key_up(Key_event code);
void input_mouse_left_click(i32 x, i32 y);
void input_mouse_left_release(i32 x, i32 y);
void input_mouse_move(i32 x, i32 y);

#endif // _WINDOW_H
