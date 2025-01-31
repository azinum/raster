// maths.h

#ifndef _MATHS_H
#define _MATHS_H

#define PI32 3.14159265359f

typedef union v2 {
  struct {
    f32 x, y;
  };
  struct {
    f32 u, v;
  };
  struct {
    f32 w, h;
  };
} v2;

// NOTE(lucas): added extra component `w` for convenience, and also alignment
typedef union v3 {
  struct {
    f32 x, y, z, w;
  };
  struct {
    f32 r, g, b, a;
  };
  struct {
    f32 pitch, yaw, roll;
  };
  // swizzling
  struct {
    v2 xy;
    v2 zw;
  };
  struct {
    f32 _;
    v2 yz;
  };
#if USE_SSE
  __m128 v;
#endif
} v3;

// TODO: make v4 an alias of v3
typedef union v4 {
  struct {
    f32 x, y, z, w;
  };
  struct {
    f32 r, g, b, a;
  };
  struct {
    f32 x1, y1, x2, y2;
  };
#if USE_SSE
  __m128 v;
#endif
} v4;

typedef v3 Plane;

typedef union m4 {
  f32 e[4][4];
#if USE_SSE
  // TODO: arm NEON SIMD support
  __m128 rows[4];
#endif
} m4;

#define V3(X, Y, Z) ((v3) { .x = (X), .y = (Y), .z = (Z), })
#define V3_OP(A, B, OP) V3((A).x OP (B).x, (A).y OP (B).y, (A).z OP (B).z)
#define V3_OP1(A, B, OP) V3((A).x OP (B), (A).y OP (B), (A).z OP (B))
#define OP(A, OPERATOR, B) (V3_OP(A, B, OPERATOR))
#define OP1(A, OPERATOR, B) (V3_OP1(A, B, OPERATOR))

#define V2(X, Y) ((v2) { .x = (X), .y = (Y) })
#define V2_OP(A, B, OP) V2((A).x OP (B).x, (A).y OP (B).y)
#define V2_OP1(A, B, OP) V2((A).x OP (B), (A).y OP (B))

extern v2 v2_zero;
extern v3 v3_zero;
extern v4 v4_zero;

#define SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define ABS(T, x) (T)(SIGN(T, x) * (x))

#define MIN3(A, B, C) (((A < B) && (A < C)) ? (A) : ((B < C) ? (B) : C ))
#define MAX3(A, B, C) (((A > B) && (A > C)) ? (A) : ((B > C) ? (B) : C ))

#define SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)

#define M4_EXPAND_COL(M, COL) M.e[COL][0], M.e[COL][1], M.e[COL][2], M.e[COL][3]
#define M4_EXPAND_ROW(M, ROW) M.e[0][ROW], M.e[1][ROW], M.e[2][ROW], M.e[3][ROW]
#define M4_EXPAND(M) M4_EXPAND_ROW(M, 0), M4_EXPAND_ROW(M, 1), M4_EXPAND_ROW(M, 2), M4_EXPAND_ROW(M, 3)
#define V2_EXPAND(A) A.x, A.y
#define V3_EXPAND(A) A.x, A.y, A.z
#define V4_EXPAND(A) A.x, A.y, A.z, A.w

#define EPS 1e-6

#ifdef NO_MATH
  extern f32 sinf(f32 n);
  extern f32 cosf(f32 n);
  extern f32 sqrtf(f32 n);
  extern f32 tanf(f32 n);
  extern f32 expf(f32 n);
  extern f32 logf(f32 n);
  extern f32 floorf(f32 n);
  extern f32 ceilf(f32 n);
  extern f32 roundf(f32 n);
#else
  #include <math.h>
#endif

extern f32 sigmoid(f32 x);
extern f32 ease_in_cubic(f32 x);
extern f32 v2_dot(v2 a, v2 b);
extern f32 v2_length(v2 a);
extern f32 v2_length_square(v2 a);
extern v2 v2_normalize(v2 a);
extern v2 v2_ease_in_cubic(v2 a);
extern m4 m4d(f32 value);
extern f32 v3_dot(v3 a, v3 b);
extern v3 v3_cross(v3 a, v3 b);
extern f32 v3_length_square(v3 a);
extern f32 v3_length(v3 a);
extern f32 v3_length_normalize(v3 a);
extern v3 v3_normalize(v3 a);
extern v3 v3_normalize_fast(v3 a);
extern v3 v3_sub(v3 a, v3 b);
extern v3 v3_div_scalar(v3 a, f32 b);
extern f32 fast_inv_sqrt(f32 a);
extern f32 f32_lerp(f32 v0, f32 v1, f32 t);
extern v2 v2_lerp(v2 a, v2 b, f32 t);
extern v3 v3_lerp(v3 a, v3 b, f32 t);
extern f32 radians(f32 angle);
extern f32 square_root(f32 a);
extern v3 m4_multiply_v3(m4 m, v3 a);
extern m4 m4_multiply(m4 a, m4 b);
extern m4 rotate(f32 angle, v3 axis);
extern m4 translate(v3 a);
extern m4 scale(v3 a);
extern m4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far);
extern m4 perspective(f32 fov, f32 aspect, f32 z_near, f32 z_far);
extern m4 look_at(v3 eye, v3 center, v3 up);
extern bool line_plane_intersection(v3 plane_pos, v3 plane_normal, v3 p1, v3 p2, f32* t);
extern f32 line_plane_intersection2(v3 a, v3 b, v3 plane);
extern f32 point_to_plane_distance(v3 plane_pos, v3 plane_normal, v3 p);
extern v3 plane_from_pos_and_normal(v3 pos, v3 normal);
extern bool point_behind_plane(v3 pos, v3 plane);
extern v3 project_to_screen(v3 p, i32 width, i32 height);

#if USE_SSE

extern m4 transpose(m4 a);
extern __m128 linear_combine(__m128 left, m4 right);

#endif

#endif // _MATHS_H
