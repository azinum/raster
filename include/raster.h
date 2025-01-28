// raster.h

#ifndef _RASTER_H
#define _RASTER_H

void init(void);
i32 raster_main(i32 argc, char** argv);
void mouse_click(i32 x, i32 y);
void input_event(i32 code, i32 type);
void update_and_render(f32 dt);
u32 display_get_width(void);
u32 display_get_height(void);
void* display_get_addr(void);
void clear_input_events(void);

#endif // _RASTER_H
