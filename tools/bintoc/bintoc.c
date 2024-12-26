// bintoc.c

#include <stdio.h>
#include <stdint.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

i32 main(i32 argc, char** argv) {
  if (argc < 3) {
    fprintf(stdout, "USAGE:\n  ./bintoc <path> <name>\n");
    return 0;
  }
  char* path = argv[1];
  char* name = argv[2];
  FILE* fp = fopen(path, "rb");
  FILE* out = stdout;
  u32 count = 0;

  #define WIDTH 14

  if (fp) {
    fprintf(out, "u8 %s[] = {\n  ", name);

    while (!feof(fp)) {
      u8 byte = 0;
      if (fread(&byte, 1, 1, fp) == 0) {
        break;
      }
      fprintf(out, "0x%.2x, ", (i32)byte);
      ++count;
      if (!(count % WIDTH)) {
        fprintf(out, "\n  ");
      }
    }
    fprintf(out, "};\n\n");
    fclose(fp);
  }
  return 0;
}
