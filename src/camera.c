// camera.c

Camera camera = {0};

void camera_init(v3 pos) {
  camera.pos = pos;
  camera.up = WORLD_UP;
  camera.right = V3(1, 0, 0);
  camera.forward = V3(0, 0, 1);

  camera.rotation = V3(0, 90, 0);
  projection = perspective(CAMERA_FOV, WINDOW_WIDTH / (f32)WINDOW_HEIGHT, CAMERA_ZNEAR, CAMERA_ZFAR);
}

void camera_update(void) {
  v3 dir = V3(
    cosf(radians(camera.rotation.yaw)) * cosf(radians(camera.rotation.pitch)),
    sinf(radians(camera.rotation.pitch)),
    sinf(radians(camera.rotation.yaw)) * cosf(radians(camera.rotation.pitch))
  );
  camera.forward = v3_normalize(dir);
  camera.right = v3_normalize(v3_cross(camera.forward, WORLD_UP));
  camera.up = v3_normalize(v3_cross(camera.right, camera.forward));

#if 0
  printf(
    "forward: %g, %g, %g - "
    "right: %g, %g, %g - "
    "up: %g, %g, %g\n"
    ,
    camera.forward.x,
    camera.forward.y,
    camera.forward.z,
    camera.right.x,
    camera.right.y,
    camera.right.z,
    camera.up.x,
    camera.up.y,
    camera.up.z
  );
#endif
  v3 center = V3_OP(camera.pos, camera.forward, -);
  view = look_at(camera.pos, center, camera.up);
}
