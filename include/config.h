// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

const u32 WINDOW_WIDTH    = 320;
const u32 WINDOW_HEIGHT   = 240;
const v3 WORLD_UP         = V3(0, 1, 0);
const f32 LIGHT_AMBIENCE  = 0.05f;
const f32 CAMERA_ZFAR     = 20.0f;
const f32 CAMERA_ZNEAR    = 0.2f;
const f32 CAMERA_FOV      = 65.0f;
const bool DITHERING      = true;
const bool FOG            = true;

#endif // _CONFIG_H
