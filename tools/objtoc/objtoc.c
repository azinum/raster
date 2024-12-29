// objtoc.c
// convert wavefront obj files into game-ready mesh format

#define COMMON_IMPLEMENTATION
#include "common.h"
#include "maths.h"
#include "mesh.h"

#include <stdio.h>
#include <stdlib.h>

//#include "test.h"

#define safe_scanf(ScanStatus, Iterator, Format, ...) { \
  i32 num_bytes_read = 0; \
  ScanStatus = sscanf(Iterator, Format "%n", __VA_ARGS__, &num_bytes_read); \
  Iterator += num_bytes_read; \
}

#define MAX_LINE_SIZE 256

typedef struct Buffer {
  u8* data;
  u32 size;
} Buffer;

Result file_read(const char* path, Buffer* buffer);
Result prepare_mesh(Buffer* buffer, Mesh* mesh, const bool sort);
Result wavefront_parse_mesh(Buffer* buffer, Mesh* mesh);
Result wavefront_sort_mesh(Mesh* mesh);
Result objtoc(Mesh* mesh, const char* name);

i32 main(i32 argc, char** argv) {
  if (argc < 3) {
    fprintf(stdout, "USAGE:\n  %s <path> <name>\n", argv[0]);
    return EXIT_FAILURE;
  }
  char* path = argv[1];
  char* name = argv[2];
  Buffer buf;
  Mesh mesh = {0};
  if (file_read(path, &buf) != Error) {
    if (prepare_mesh(&buf, &mesh, true) != Error) {
      if (wavefront_parse_mesh(&buf, &mesh) != Error) {
        if (wavefront_sort_mesh(&mesh) != Error) {
          objtoc(&mesh, name);
        }
      }
    }
  }
  return EXIT_SUCCESS;
}

Result file_read(const char* path, Buffer* buffer) {
  Result result = Ok;
  u32 num_bytes_read = 0;
  u32 size = 0;
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "file_read: file `%s` does not exist.\n", path);
    return_defer(Error);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buffer->data = (u8*)malloc(size * sizeof(u8));
  buffer->size = size;
  if (!buffer->data) {
    buffer->size = 0;
    return_defer(Error);
  }

  num_bytes_read = fread(buffer->data, 1, size, fp);
  if (num_bytes_read != size) {
    fprintf(stderr, "file_read: failed to read file `%s`.\n", path);
    return_defer(Error);
  } 
defer:
  return Ok;
}

Result prepare_mesh(Buffer* buffer, Mesh* mesh, const bool sort) {
  Result result = Ok;
  i32 scan_status = 0;
  char line[MAX_LINE_SIZE] = {0};
  char* iter = (char*)&buffer->data[0];
  do {
    safe_scanf(scan_status, iter, "%s", line);
    if (scan_status == EOF) {
      break;
    }

    if (!strncmp(line, "v", MAX_LINE_SIZE)) {
      mesh->vertex_count++;
    }
    else if (!strncmp(line, "vt", MAX_LINE_SIZE)) {
      mesh->uv_count++;
    }
    else if (!strncmp(line, "vn", MAX_LINE_SIZE)) {
      mesh->normal_count++;
    }
    else if (!strncmp(line, "f", MAX_LINE_SIZE)) {
      mesh->vertex_index_count += 3;
      mesh->uv_index_count += 3;
      mesh->normal_index_count += 3;
    }
  } while (1);

  if (sort) {
    mesh->uv_count = mesh->uv_index_count;
    mesh->normal_count = mesh->normal_index_count;
  }
  const u32 size = sizeof(v3) * mesh->vertex_count +
    sizeof(u32) * mesh->vertex_index_count +
    sizeof(v2) * mesh->uv_count +
    sizeof(u32) * mesh->uv_index_count +
    sizeof(v3) * mesh->normal_count +
    sizeof(u32) * mesh->normal_index_count;
  void* data = malloc(size);

  mesh->vertex = (void*)data;
  mesh->vertex_index = (void*)((u8*)mesh->vertex + (sizeof(v3) * mesh->vertex_count));

  mesh->uv = (void*)((u8*)mesh->vertex_index + (sizeof(u32) * mesh->vertex_index_count));
  mesh->uv_index = (void*)((u8*)mesh->uv + (sizeof(v2) * mesh->uv_count));

  mesh->normal = (void*)((u8*)mesh->uv_index + (sizeof(u32) * mesh->uv_index_count));
  mesh->normal_index = (void*)((u8*)mesh->normal + (sizeof(v3) * mesh->normal_count));
  return result;
}

Result wavefront_parse_mesh(Buffer* buffer, Mesh* mesh) {
  Result result = Ok;
  i32 scan_status = 0;
  char line[MAX_LINE_SIZE] = {0};
  char* iter = (char*)&buffer->data[0];

  v3* vertex = &mesh->vertex[0];
  u32* vertex_index = &mesh->vertex_index[0];
  v2* uv = &mesh->uv[0];
  u32* uv_index = &mesh->uv_index[0];
  v3* normal = &mesh->normal[0];
  u32* normal_index = &mesh->normal_index[0];

  do {
    safe_scanf(scan_status, iter, "%s", line);
    if (scan_status == EOF) {
      break;
    }

    if (!strncmp(line, "v", MAX_LINE_SIZE)) {
      v3 v;
      safe_scanf(scan_status, iter, "%f %f %f", &v.x, &v.y, &v.z);
      *vertex++ = v;
    }
    else if (!strncmp(line, "vt", MAX_LINE_SIZE)) {
      v2 v;
      safe_scanf(scan_status, iter, "%f %f", &v.x, &v.y);
      *uv++ = v;
    }
    else if (!strncmp(line, "vn", MAX_LINE_SIZE)) {
      v3 v;
      safe_scanf(scan_status, iter, "%f %f %f", &v.x, &v.y, &v.z);
      *normal++ = v;
    }
    else if (!strncmp(line, "f", MAX_LINE_SIZE)) {
      u32 vi[3] = {0};  // Vertex indices
      u32 ui[3] = {0};  // UV indices
      u32 ni[3] = {0};  // Normal indices
      safe_scanf(scan_status, iter,
        "%u/%u/%u %u/%u/%u %u/%u/%u",
        &vi[0], &ui[0], &ni[0],
        &vi[1], &ui[1], &ni[1],
        &vi[2], &ui[2], &ni[2]
      );
      if (scan_status != 9) {
        fprintf(stderr, "wavefront_parse_mesh: failed to parse wavefront object file.\n");
        return_defer(Error);
      }
      *vertex_index++ = vi[0] - 1;
      *vertex_index++ = vi[1] - 1;
      *vertex_index++ = vi[2] - 1;

      *uv_index++ = ui[0] - 1;
      *uv_index++ = ui[1] - 1;
      *uv_index++ = ui[2] - 1;

      *normal_index++ = ni[0] - 1;
      *normal_index++ = ni[1] - 1;
      *normal_index++ = ni[2] - 1;
    }
  } while (1);
defer:
  return result; 
}

Result wavefront_sort_mesh(Mesh* mesh) {
  Result result = Ok;

  u32 uv_count = mesh->uv_count;
  v2* uv = malloc(uv_count * sizeof(v2));;
  u32 normal_count = mesh->normal_count;
  v3* normal = malloc(normal_count * sizeof(v3));
  if (uv && normal) {
    memcpy(uv, mesh->uv, uv_count * sizeof(v2));
    memcpy(normal, mesh->normal, normal_count * sizeof(v3));

    for (u32 i = 0; i < mesh->vertex_index_count; ++i) {
      u32 index = mesh->vertex_index[i];
      u32 uv_index = mesh->uv_index[i];
      (void)uv_index;
      u32 normal_index = mesh->normal_index[i];

      // mesh->uv[i] = uv[uv_index];
      mesh->normal[index] = normal[normal_index];
    }

    free(uv);
    free(normal);
  }
  else {
    result = Error;
  }
  return result;
}

Result objtoc(Mesh* mesh, const char* name) {
  printf("v3 %s_vertex[] = {", name);
  for (u32 i = 0; i < mesh->vertex_count; ++i) {
    v3 v = mesh->vertex[i];
    printf("{%.04f,%.04f,%.04f, 1},", v.x, v.y, v.z);
  }
  printf("};\n");

  printf("v3 %s_normal[] = {", name);
  for (u32 i = 0; i < mesh->normal_count; ++i) {
    v3 v = mesh->normal[i];
    printf("{%.04f,%.04f,%.04f},", v.x, v.y, v.z);
  }
  printf("};\n");

  printf("v2 %s_uv[] = {", name);
  for (u32 i = 0; i < mesh->uv_count; ++i) {
    v2 v = mesh->uv[i];
    printf("{%.04f,%.04f},", v.x, v.y);
  }
  printf("};\n");

  printf("u32 %s_vertex_index[] = {", name);
  for (u32 i = 0; i < mesh->vertex_index_count; ++i) {
    u32 v = mesh->vertex_index[i];
    printf("%u,", v);
  }
  printf("};\n");

  printf("u32 %s_normal_index[] = {", name);
  for (u32 i = 0; i < mesh->normal_index_count; ++i) {
    u32 v = mesh->normal_index[i];
    printf("%u,", v);
  }
  printf("};\n");

  printf("u32 %s_uv_index[] = {", name);
  for (u32 i = 0; i < mesh->uv_index_count; ++i) {
    u32 v = mesh->uv_index[i];
    printf("%u,", v);
  }
  printf("};\n");

  printf(
    "Mesh %s = {\n"
    "  .vertex_count = %u,\n"
    "  .vertex_index_count = %u,\n"
    "  .normal_count = %u,\n"
    "  .normal_index_count = %u,\n"
    "  .uv_count = %u,\n"
    "  .uv_index_count = %u,\n"

    "  .vertex = %s_vertex,\n"
    "  .vertex_index = %s_vertex_index,\n"
    "  .normal = %s_normal,\n"
    "  .normal_index = %s_normal_index,\n"
    "  .uv = %s_uv,\n"
    "  .uv_index = %s_uv_index,\n"
    "};\n"
    ,
    name,
    mesh->vertex_count,
    mesh->vertex_index_count,
    mesh->normal_count,
    mesh->normal_index_count,
    mesh->uv_count,
    mesh->uv_index_count,
    name,
    name,
    name,
    name,
    name,
    name
  );
  return Ok;
}
