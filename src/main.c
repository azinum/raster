// main.c

#include "raster.c"

#ifndef TARGET_WASM

i32 main(i32 argc, char** argv) {
  init();
  return raster_main(argc, argv);
}

#else

void wasm_main(void) {
  init();
}

#endif
