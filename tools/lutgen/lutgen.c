// lutgen.c

#include <math.h>

#define COMMON_IMPLEMENTATION
#include "common.h"

i32 resolution = 1024;
#define TAU32 (2 * PI32)

static void print_lut(i32 fd, const char* name, f32 range, f32 (*f)(f32));

i32 main(i32 argc, char** argv) {
  if (argc > 1) {
    resolution = atoi(argv[1]);
  }
  print_lut(STDOUT_FILENO, "sin_lut", TAU32, sinf);
  print_lut(STDOUT_FILENO, "cos_lut", TAU32, cosf);
  return EXIT_SUCCESS;
}

void print_lut(i32 fd, const char* name, f32 range, f32 (*f)(f32)) {
  dprintf(fd, "// %zu kb\n", (resolution * sizeof(f32)) / 1024);
  dprintf(fd, "f32 %s[%d] = { ", name, resolution);
  for (size_t i = 0; i < resolution; ++i) {
    f32 n = f(range * (i / (f32)resolution));
    dprintf(fd, "%ff,", n);
  }
  dprintf(fd, " };\n");
}
