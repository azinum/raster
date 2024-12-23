// maths.h

#ifndef _MATHS_H
#define _MATHS_H

#define PI32 3.14159265359f

typedef union v3 {
  struct {
    f32 x, y, z;
  };
  struct {
    f32 r, g, b;
  };
  struct {
    f32 pitch, yaw, roll;
  };
} v3;

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

typedef union m4 {
  f32 e[4][4];
#if USE_SSE
  __m128 rows[4];
#endif
} m4;

#define V3(X, Y, Z) ((v3) { .x = (X), .y = (Y), .z = (Z) })
#define V3_OP(A, B, OP) V3((A).x OP (B).x, (A).y OP (B).y, (A).z OP (B).z)
#define V3_OP1(A, B, OP) V3((A).x OP (B), (A).y OP (B), (A).z OP (B))

#define V2(X, Y) ((v2) { .x = (X), .y = (Y) })
#define V2_OP(A, B, OP) V2((A).x OP (B).x, (A).y OP (B).y)
#define V2_OP1(A, B, OP) V2((A).x OP (B), (A).y OP (B))

#define v2_zero V2(0, 0)
#define v3_zero V3(0, 0, 0)

#ifdef NO_MATH
  extern f32 sinf(f32 n);
  extern f32 cosf(f32 n);
  extern f32 sqrtf(f32 n);
  extern f32 tanf(f32 n);
  extern f32 expf(f32 n);
#else
  #include <math.h>
#endif

extern f32 sigmoid(f32 x);
extern f32 ease_in_cubic(f32 x);
extern v2 v2_ease_in_cubic(v2 a);
extern m4 m4d(f32 value);
extern f32 v3_dot(v3 a, v3 b);
extern v3 v3_cross(v3 a, v3 b);
extern f32 v3_length_square(v3 a);
extern f32 v3_length(v3 a);
extern f32 v3_length_normalize(v3 a);
extern v3 v3_normalize(v3 a);
extern v3 v3_normalize_fast(v3 a);
extern f32 fast_inv_sqrt(f32 a);
extern f32 lerp(f32 v0, f32 v1, f32 t);
extern v3 v3_lerp(v3 a, v3 b, f32 t);
extern f32 radians(f32 angle);
extern f32 square_root(f32 a);
extern m4 m4_multiply(m4 a, m4 b);
extern m4 rotate(f32 angle, v3 axis);
extern m4 translate(v3 a);
extern m4 scale(v3 a);
extern m4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far);
extern m4 perspective(f32 fov, f32 aspect, f32 z_near, f32 z_far);
extern m4 look_at(v3 eye, v3 center, v3 up);
#if USE_SSE

extern m4 transpose(m4 a);
extern __m128 linear_combine(__m128 left, m4 right);

#endif

#endif // _MATHS_H
