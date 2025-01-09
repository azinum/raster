// camera.c

Camera camera = {0};

void camera_init(v3 pos) {
  camera.pos = pos;
  camera.up = WORLD_UP;
  camera.right = V3(1, 0, 0);
  camera.forward = V3(0, 0, 1);

  camera.rotation = V3(0, -90, 0);
  projection = perspective(CAMERA_FOV, WINDOW_WIDTH / (f32)WINDOW_HEIGHT, CAMERA_ZNEAR, CAMERA_ZFAR);
}

void camera_update(void) {
  v3 dir = V3(
    cosf(radians(camera.rotation.yaw)) * cosf(radians(camera.rotation.pitch)),
    sinf(radians(camera.rotation.pitch)),
    sinf(radians(camera.rotation.yaw)) * cosf(radians(camera.rotation.pitch))
  );
#if 0
  camera.forward = v3_normalize(dir);
  camera.right = v3_normalize(v3_cross(WORLD_UP, camera.forward));
  camera.up = v3_normalize(v3_cross(camera.forward, camera.right));

  v3 center = V3_OP(camera.pos, camera.forward, +);
  view = look_at(camera.pos, center, camera.up);
#else
  camera.forward = v3_normalize(dir);
  camera.right = v3_normalize(v3_cross(WORLD_UP, camera.forward));
  camera.up = v3_normalize(v3_cross(camera.right, camera.forward));

  v3 center = V3_OP(camera.pos, camera.forward, +);
  view = look_at(camera.pos, center, camera.up);
#endif
}
