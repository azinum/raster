// font2c.c
// grabbed from pull request by github.com/derSicking: https://github.com/tsoding/olive.c/pull/25/commits/4c7f440b49be4913d1bdccbe9f01b895f9bcc717

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FROM_CHAR (size_t)32
#define TO_CHAR (size_t)127

void determine_bitmap_sizes(FT_Face face, size_t* width, size_t* height, size_t* baseline) {
  // Iterate over all desired glyphs
  // find the maximum ascender height, by checking the glyphs bitmap_top value
  // find the maximum descender depth, by checking bitmap_top - height
  // height of all bitmaps has to be max ascender + max descender (+1? Is baseline included in asc or desc? or none?)

  // width of all bitmaps has to be the max width of any glyph
  // TODO: align glyphs using bitmap_left
  FT_Int max_ascender = 0;
  FT_Int max_descender = 0;
  FT_UInt max_width = 0;

  for (unsigned char c = FROM_CHAR; c < TO_CHAR; ++c) {	
    int glyph_index = FT_Get_Char_Index(face, c);

    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
      fprintf(stderr, "There was a problem loading the glyph for `%c`\n", c);
      continue;
    }

    if (face->glyph->bitmap_top > max_ascender) {
      max_ascender = face->glyph->bitmap_top;
    }
    if (((FT_Int)face->glyph->bitmap.rows - face->glyph->bitmap_top) > max_descender) {
      max_descender = (face->glyph->bitmap.rows - face->glyph->bitmap_top);
    }
    if (face->glyph->bitmap.width > max_width) {
      max_width = face->glyph->bitmap.width;
    }
  }

  *height = (size_t) (max_ascender + max_descender);
  *width = (size_t) max_width;
  *baseline = (size_t) max_ascender;
}

void write_byte_array_code_for_char_to_file(FILE* out, FT_Face face, const unsigned char c, const size_t width, const size_t height, const size_t baseline) {
  int glyph_index = FT_Get_Char_Index(face, c);

  if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)){
    fprintf(stderr, "There was a problem loading the glyph for %c\n", c);
    // TODO: return a fallback, like missing character box
    return;
	}

  // work out how many lines to skip at start
  int skip_lines = (int)baseline - face->glyph->bitmap_top;
  if (skip_lines < 0) {
    skip_lines = 0;
  }

  fprintf(out, " [%d] = { // `%c`\n", c, c);
  FT_Bitmap* bitmap = &face->glyph->bitmap;
  for (size_t y = 0; y < height; ++y) {
    fprintf(out, "        {");
    if (y < (unsigned int) skip_lines || y >= (unsigned int) (skip_lines + bitmap->rows)) {
      for (size_t x = 0; x < width; ++x) {
        fprintf(out, "0, ");
      }
    }
    else {
      for (size_t x = 0; x < width; ++x) {
        if (x >= bitmap->width) {
          fprintf(out, "0, ");
          continue;
        }
        fprintf(out, "%c, ", bitmap->buffer[bitmap->width * (y - skip_lines) + x] == 0 ? '0' : '1');
      }
    }
    fprintf(out, "},\n");
  }
  fprintf(out, "    },\n");
}

int generate_c_file_from_ttf(const char* input_file_path, const char* output_file_path, const char* name) {
  FT_Library library;
  FT_Face face;

  int error = FT_Init_FreeType(&library);
  if (error) {
    fprintf(stderr, "ERROR: there was a problem initializing the FreeType library\n");
    return EXIT_FAILURE;
  }

  // TODO: allow user to choose a face_index other than 0
  // TODO: check for available face_indices by setting face_index to -1 and checking face->num_faces
  error = FT_New_Face(library, input_file_path, 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "ERROR: font file %s: format is not supported\n", input_file_path);
    return EXIT_FAILURE;
  }
  else if (error || face == NULL) {
    fprintf(stderr, "ERROR: font file %s could not be read\n", input_file_path);
    return EXIT_FAILURE;
  }

  // FT_Set_Char_Size(
  //   FT_Face     face,
  //   FT_F26Dot6  char_width,
  //   FT_F26Dot6  char_height,
  //   FT_UInt     horz_resolution,
  //   FT_UInt     vert_resolution);
  // TODO: let user specify a resolution and font size
  // error = FT_Set_Char_Size(face, 0, 16*64, 64, 64);
  error = FT_Set_Char_Size(face, 0, 12*48, 64, 64);

  size_t width = 0,
         height = 0,
         baseline = 0;
  determine_bitmap_sizes(face, &width, &height, &baseline);

  FILE* out = NULL;
  if (output_file_path) {
    out = fopen(output_file_path, "wb");
    if (out == NULL) {
      fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", output_file_path, strerror(errno));
      return EXIT_FAILURE;
    }
  }
  else {
    out = stdout;
  }

  fprintf(out, "static char %s_glyphs[%zu][%zu][%zu] = {\n", name, TO_CHAR, height, width);

  for (unsigned char c = FROM_CHAR; c < TO_CHAR; ++c) {
    write_byte_array_code_for_char_to_file(out, face, c, width, height, baseline);
  }

  fprintf(out, "};\n");
  fprintf(out, "static const size_t %s_width = %zu;\n", name, width);
  fprintf(out, "static const size_t %s_height = %zu;\n", name, height);
  fprintf(out, "Font %s = { .glyphs = (u8*)%s_glyphs, .count = %zu, .width = %zu, .height = %zu, };\n", name, name, TO_CHAR, width, height);
	return 0;
}

const char *shift(int* argc, char*** argv) {
  assert(*argc > 0);
  const char* result = *argv[0];
  *argc -= 1;
  *argv += 1;
  return result;
}

void usage(FILE* out, const char* program_name) {
  fprintf(out, "Usage: %s [OPTIONS] <input/file/path.ttf>\n", program_name);
  fprintf(out, "OPTIONS:\n");
  fprintf(out, "  -o <output/file/path.h>\n");
  fprintf(out, "  -n <name>\n");
  // TODO: fprintf(out, "    -i <font face index>\n");
}

int main(int argc, char** argv) {
  assert(argc > 0);
  const char* program_name = shift(&argc, &argv);
  const char* output_file_path = NULL;
  const char* input_file_path = NULL;
  const char* name = NULL;

  while (argc > 0) {
    const char* flag = shift(&argc, &argv);
    if (strcmp(flag, "-o") == 0) {
      if (argc <= 0) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no value is provided for flag %s\n", flag);
        return EXIT_FAILURE;
      }

      if (output_file_path != NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: %s was already provided\n", flag);
        return EXIT_FAILURE;
      }

      output_file_path = shift(&argc, &argv);
    }
    else if (strcmp(flag, "-n") == 0) {
      if (argc <= 0) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no value is provided for flag %s\n", flag);
        return EXIT_FAILURE;
      }

      if (name != NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: %s was already provided\n", flag);
        return EXIT_FAILURE;
      }

      name = shift(&argc, &argv);
    }
    else {
      if (input_file_path != NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: input file path was already provided\n");
        return EXIT_FAILURE;
      }
      input_file_path = flag;
    }
  }

  if (input_file_path == NULL) {
    usage(stderr, program_name);
    fprintf(stderr, "ERROR: expected input file path\n");
    return EXIT_FAILURE;
  }

  if (name == NULL) {
    // TODO: infer a fitting name from input path
    name = "font";
  }
  else {
    size_t n = strlen(name);
    if (n == 0) {
      fprintf(stderr, "ERROR: name cannot be empty\n");
      return EXIT_FAILURE;
    }

    if (isdigit(name[0])) {
      fprintf(stderr, "ERROR: name cannot start from a digit\n");
      return EXIT_FAILURE;
    }

    for (size_t i = 0; i < n; ++i) {
      if (!isalnum(name[i]) && name[i] != '_') {
        fprintf(stderr, "ERROR: name can only contains alphanumeric characters and underscores\n");
        return EXIT_FAILURE;
      }
    }
  }
	return generate_c_file_from_ttf(input_file_path, output_file_path, name);
}

