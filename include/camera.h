// camera.h

#ifndef _CAMERA_H
#define _CAMERA_H

m4 view = {0};
m4 projection = {0};

typedef struct Camera {
  v3 pos;
  v3 up;
  v3 right;
  v3 forward;

  v3 rotation;
} Camera;

extern Camera camera;

void camera_init(v3 pos);
void camera_update(void);

#endif // _CAMERA_H
