// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#define DEBUG_OUT_OF_BOUNDS

#define RASTER_WIDTH      (320)
#define RASTER_HEIGHT     (240)
#define WINDOW_WIDTH      (800)
#define WINDOW_HEIGHT     (600)
const v3 WORLD_UP         = V3(0, 1, 0);
f32 LIGHT_AMBIENCE        = 1.0f / (f32)UINT8_MAX;
f32 CAMERA_ZFAR           = 35.0f;
f32 CAMERA_ZNEAR          = 0.8f;
f32 CAMERA_FOV            = 65.0f;
bool DITHERING            = false;
bool FOG                  = false;
bool EDGE_DETECTION       = false;
bool RENDER_VERTICES      = false;
Color FOG_COLOR           = COLOR_RGB(0, 0, 0);
Color EDGE_DETECTION_COLOR = COLOR_RGB(0, 0, 0);
v3 VOXELGI_POS            = V3(7, 0, 12);
bool VOXELGI_RENDER_VOXELS = false; // for debugging
#define VOXELGI_X         16
#define VOXELGI_Y         4
#define VOXELGI_Z         12
#define VOXELGI_VOXEL_COUNT (VOXELGI_X * VOXELGI_Y * VOXELGI_Z)
const f32 DT_MIN          = 1.0f / 1000.0f;
const f32 DT_MAX          = 1.0f / 10.0f;

#endif // _CONFIG_H
