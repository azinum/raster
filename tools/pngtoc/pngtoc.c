// pngtoc.c

#include <assert.h>
#include <math.h>

#define COMMON_IMPLEMENTATION
#include "common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* next(i32* argc, char*** argv);
Result png2c(FILE* fp, const char* path, const char* name);
Result pixels2c(FILE* fp, u32* data, i32 x, i32 y, const char* name);

i32 main(i32 argc, char** argv) {
  i32 result = EXIT_SUCCESS;
  if (argc < 3) {
    printf("Usage; %s <path/to/image.png> <name>\n", argv[0]);
    return_defer(EXIT_FAILURE);
  }
  next(&argc, &argv);

  const char* path = next(&argc, &argv);
  const char* name = next(&argc, &argv);

  FILE* fp = stdout;
  if (png2c(fp, path, name) != Ok) {
    return_defer(EXIT_FAILURE);
  }
defer:
  return result;
}

const char* next(i32* argc, char*** argv) {
  assert(*argc > 0);
  const char* result = *argv[0];
  *argc -= 1;
  *argv += 1;
  return result;
}

Result png2c(FILE* fp, const char* path, const char* name) {
  Result result = Ok;
  u32* data = NULL;

  i32 x = 0;
  i32 y = 0;
  data = (u32*)stbi_load(path, &x, &y, NULL, 4);
  if (!data) {
    fprintf(stderr, "error: failed to load image `%s`\n", path);
    return_defer(Error);
  }
  return_defer(pixels2c(fp, data, x, y, name));
defer:
  if (data) {
    stbi_image_free(data);
  }
  return result;
}

Result pixels2c(FILE* fp, u32* data, i32 x, i32 y, const char* name) {
  Result result = Ok;
  fprintf(fp, "u32 %s_pixels[] = {\n", name);
  u32 index = 0;
  for (u32 py = 0; py < y; ++py) {
    for (u32 px = 0; px < x; ++px) {
      fprintf(fp, "0x%08x,", data[index]);
      ++index;
    }
  }
  fprintf(fp, "};\n");
  fprintf(fp, "Texture %s = { .data = (struct Color*)%s_pixels, .width = %d, .height = %d, };\n", name, name, x, y);
  return result;
}

