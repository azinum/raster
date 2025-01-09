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
bool DITHERING      = false;
bool FOG            = false;
bool EDGE_DETECTION = false;
Color FOG_COLOR     = COLOR_RGB(0, 0, 0);
Color EDGE_DETECTION_COLOR = COLOR_RGB(0, 0, 0);
v3 VOXELGI_POS      = V3(7, 1.5f, 12);
bool VOXELGI_RENDER_VOXELS = false; // for debugging
const i32 VOXELGI_X = 16;
const i32 VOXELGI_Y = 4;
const i32 VOXELGI_Z = 12;
#define VOXELGI_VOXEL_COUNT (VOXELGI_X * VOXELGI_Y * VOXELGI_Z)

#endif // _CONFIG_H
