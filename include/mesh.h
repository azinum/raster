// mesh.h

#ifndef _MESH_H
#define _MESH_H

typedef struct Mesh {
  u32 vertex_count;
  u32 index_count;
  u32 normal_count;
  u32 normal_index_count;
  u32 uv_count;
  u32 uv_index_count;

  v3* vertex;
  u32* vertex_index;
  v3* normal;
  u32* normal_index;
  v2* uv;
  u32* uv_index;
} Mesh;

#endif // _MESH_H
