// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#define DEBUG_OUT_OF_BOUNDS

const u32 WINDOW_WIDTH    = 320;
const u32 WINDOW_HEIGHT   = 240;
const v3 WORLD_UP         = V3(0, 1, 0);
f32 LIGHT_AMBIENCE  = 1.0f / (f32)UINT8_MAX;
f32 CAMERA_ZFAR     = 50.0f;
f32 CAMERA_ZNEAR    = 0.2f;
f32 CAMERA_FOV      = 65.0f;
bool DITHERING      = true;
bool FOG            = true;
bool EDGE_DETECTION = false;
Color FOG_COLOR     = COLOR_RGB(0, 0, 0);
Color EDGE_DETECTION_COLOR = COLOR_RGB(0, 0, 0);
v3 VOXELGI_POS      = V3(7, 0.5f, 12);

#endif // _CONFIG_H
