// maths.c

v2 v2_zero = (v2) { .x = 0, .y = 0, };
v3 v3_zero = (v3) { .x = 0, .y = 0, .z = 0, .w = 0, };
v4 v4_zero = (v4) { .x = 0, .y = 0, .z = 0, .w = 0, };

inline f32 sigmoid(f32 x) {
  return (0.5f / (0.5f + expf(-x)));
}

inline f32 ease_in_cubic(f32 x) {
  return x * x * x;
}

inline f32 v2_dot(v2 a, v2 b) {
  return (a.x * b.x) + (a.y * b.y);
}

inline f32 v2_length(v2 a) {
  return square_root(v2_length_square(a));
}

inline v2 v2_normalize(v2 a) {
  v2 result = {0};
  f32 length = v2_length(a);
  if (length != 0) {
    f32 inv = 1.0f / length;
    result.x = a.x * inv;
    result.y = a.y * inv;
  }
  return result;
}

inline f32 v2_length_square(v2 a) {
  return v2_dot(a, a);
}

inline v2 v2_ease_in_cubic(v2 a) {
  return V2(
    ease_in_cubic(a.x),
    ease_in_cubic(a.y)
  );
}

inline m4 m4d(float value) {
  m4 result = {0};

  result.e[0][0] = value;
  result.e[1][1] = value;
  result.e[2][2] = value;
  result.e[3][3] = value;

  return result;
}

inline f32 v3_dot(v3 a, v3 b) {
#ifdef USE_SSE
  a.v = _mm_mul_ps(a.v, b.v);
  return a.x + a.y + a.z;
#else
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
#endif
}

inline v3 v3_cross(v3 a, v3 b) {
  return V3(
    (a.y * b.z) - (a.z * b.y),
    (a.z * b.x) - (a.x * b.z),
    (a.x * b.y) - (a.y * b.x)
  );
}

inline f32 v3_length_square(v3 a) {
  return v3_dot(a, a);
}

inline f32 v3_length(v3 a) {
  return square_root(v3_length_square(a));
}

inline f32 v3_length_normalize(v3 a) {
  return fast_inv_sqrt(v3_length_square(a));
}

inline v3 v3_normalize(v3 a) {
  v3 result = {0};
  f32 length = v3_length(a);
  if (length != 0) {
    f32 inv = 1.0f / length;
    result.x = a.x * inv;
    result.y = a.y * inv;
    result.z = a.z * inv;
  }
  return result;
}

inline v3 v3_normalize_fast(v3 a) {
  f32 length = v3_length_normalize(a);

  return V3(
    a.x * length,
    a.y * length,
    a.z * length
  );
}

inline v3 v3_sub(v3 a, v3 b) {
  return V3(
    a.x - b.x,
    a.y - b.y,
    a.z - b.z
  );
}

inline v3 v3_div_scalar(v3 a, f32 b) {
  if (b != 0) {
    return (v3) {{
      a.x / b,
      a.y / b,
      a.z / b,
      a.w,
    }};
  }
  return a;
}

inline f32 fast_inv_sqrt(f32 a) {
  union { f32 f; i32 i; } i, y;
  i.i = 0;
  y.f = a;

  const f32 three_halfs = 1.5f;
  f32 x2 = a * 0.5f;

  i.f = y.f;
  i.i = 0x5f3759df - (i.i >> 1);
  y.i = i.i;

  y.f = y.f * (three_halfs - (x2 * y.f * y.f));
  return y.f;
}

inline float f32_lerp(float v0, float v1, float t) {
  return (1.0f - t) * v0 + t * v1;
}

inline v2 v2_lerp(v2 a, v2 b, f32 t) {
  return (v2) {
    f32_lerp(a.x, b.x, t),
    f32_lerp(a.y, b.y, t)
  };
}

inline v3 v3_lerp(v3 a, v3 b, f32 t) {
#ifdef USE_SSE
  v3 result;
  result.v = _mm_add_ps(
    _mm_mul_ps(
      _mm_set1_ps(t),
      _mm_sub_ps(b.v, a.v)
    ),
    a.v
  );
  return result;
#else
  return V3(
    f32_lerp(a.x, b.x, t),
    f32_lerp(a.y, b.y, t),
    f32_lerp(a.z, b.z, t)
  );
#endif
}

inline f32 radians(f32 angle) {
  return angle * (PI32 / 180.0f);
}

inline f32 square_root(f32 a) {
  f32 result = 0;
#ifdef USE_SSE
  result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(a)));
#else
  result = sqrtf(a);
#endif
  return result;
}

inline v3 m4_multiply_v3(m4 m, v3 a) {
  f32 x = a.x;
  f32 y = a.y;
  f32 z = a.z;
  return (v3) {
    x * m.e[0][0] + y * m.e[1][0] + z * m.e[2][0] + m.e[3][0],
    x * m.e[0][1] + y * m.e[1][1] + z * m.e[2][1] + m.e[3][1],
    x * m.e[0][2] + y * m.e[1][2] + z * m.e[2][2] + m.e[3][2],
    x * m.e[0][3] + y * m.e[1][3] + z * m.e[2][3] + m.e[3][3],
  };
}

inline m4 m4_multiply(m4 a, m4 b) {
  m4 result = {0};

#ifdef USE_SSE
  m4 left = transpose(a);
  m4 right = transpose(b);

  result.rows[0] = linear_combine(left.rows[0], right);
  result.rows[1] = linear_combine(left.rows[1], right);
  result.rows[2] = linear_combine(left.rows[2], right);
  result.rows[3] = linear_combine(left.rows[3], right);

  result = transpose(result);
#else
  for (i32 col = 0; col < 4; ++col) {
    for (i32 row = 0; row < 4; ++row) {
      float sum = 0;
      for (i32 current = 0; current < 4; ++current) {
        sum += a.e[current][row] * b.e[col][current];
      }
      result.e[col][row] = sum;
    }
  }
#endif
  return result;
}

inline m4 rotate(f32 angle, v3 axis) {
  m4 result = m4d(1.0f);

  axis = v3_normalize_fast(axis);

  f32 sin_theta = sinf(radians(angle));
  f32 cos_theta = cosf(radians(angle));
  f32 cos_value = 1.0f - cos_theta;

  result.e[0][0] = (axis.x * axis.x * cos_value) + cos_theta;
  result.e[0][1] = (axis.x * axis.y * cos_value) + (axis.z * sin_theta);
  result.e[0][2] = (axis.x * axis.z * cos_value) - (axis.y * sin_theta);

  result.e[1][0] = (axis.y * axis.x * cos_value) - (axis.z * sin_theta);
  result.e[1][1] = (axis.y * axis.y * cos_value) + cos_theta;
  result.e[1][2] = (axis.y * axis.z * cos_value) + (axis.x * sin_theta);

  result.e[2][0] = (axis.z * axis.x * cos_value) + (axis.y * sin_theta);
  result.e[2][1] = (axis.z * axis.y * cos_value) - (axis.x * sin_theta);
  result.e[2][2] = (axis.z * axis.z * cos_value) + cos_theta;

  return result;
}

inline m4 translate(v3 a) {
  m4 result = m4d(1.0f);

  result.e[3][0] = a.x;
  result.e[3][1] = a.y;
  result.e[3][2] = a.z;

  return result;
}

inline m4 scale(v3 a) {
  m4 result = {0};

  result.e[0][0] = a.x;
  result.e[1][1] = a.y;
  result.e[2][2] = a.z;
  result.e[3][3] = 1.0f;

  return result;
}

inline m4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far) {
  m4 result = {0};

  result.e[0][0] = 2.0f / (right - left);
  result.e[1][1] = 2.0f / (top - bottom);
  result.e[2][2] = 2.0f / (z_near - z_far);
  result.e[3][3] = 1.0f;

  result.e[3][0] = (left + right) / (left - right);
  result.e[3][1] = (bottom + top) / (bottom - top);
  result.e[3][2] = (z_far + z_near) / (z_near - z_far);

  return result;
}

inline m4 perspective(f32 fov, f32 aspect, f32 z_near, f32 z_far) {
  m4 result = {0};
  float tan_theta_over2 = tanf(fov * (PI32 / 360.0f));

  result.e[0][0] = 1.0f / tan_theta_over2;
  result.e[1][1] = aspect / tan_theta_over2;
  result.e[2][3] = -1.0f;
  result.e[2][2] = (z_near + z_far) / (z_near - z_far);
  result.e[3][2] = (1.0f * z_near * z_far) / (z_near - z_far);
  result.e[3][3] = 0.0f;

  return result;
}

inline m4 look_at(v3 eye, v3 center, v3 up) {
	m4 result = m4d(1.0f);

	v3 front = v3_normalize_fast(V3_OP(center, eye, -));
	v3 side = v3_normalize_fast(v3_cross(front, up));
	v3 u = v3_normalize_fast(v3_cross(side, front));

	result.e[0][0] = side.x;
	result.e[0][1] = u.x;
	result.e[0][2] = -front.x;

	result.e[1][0] = side.y;
	result.e[1][1] = u.y;
	result.e[1][2] = -front.y;

	result.e[2][0] = side.z;
	result.e[2][1] = u.z;
	result.e[2][2] = -front.z;

	result.e[3][0] = -v3_dot(side, eye);
	result.e[3][1] = -v3_dot(u, eye);
	result.e[3][2] = v3_dot(front, eye);
	result.e[3][3] = 1.0f;

	return result;
}

inline bool line_plane_intersection(v3 plane_pos, v3 plane_normal, v3 p1, v3 p2, f32* t) {
  f32 plane_distance = v3_dot(plane_normal, plane_pos);
  f32 a_dot = v3_dot(p1, plane_normal);
  f32 b_dot = v3_dot(p2, plane_normal);
  f32 ba_diff = b_dot - a_dot;
  if (ba_diff == 0) {
    *t = 0;
    return false;
  }
  *t = (plane_distance - a_dot) / ba_diff;
  return true;
}

// big thanks to getintogamedev
// https://www.youtube.com/watch?v=VQHwqMz2Ef8
inline f32 line_plane_intersection2(v3 a, v3 b, v3 plane) {
  f32 denom = (plane.x * (b.x - a.x) + plane.y * (b.y - a.y) + plane.z * (b.z - a.z));
  if (denom != 0) {
    return -(a.x * plane.x + a.y * plane.y + a.z * plane.z + plane.w) / denom;
  }
  return 0;
}

inline f32 point_to_plane_distance(v3 plane_pos, v3 plane_normal, v3 p) {
  return v3_dot(plane_normal, p) - v3_dot(plane_normal, plane_pos);
}

inline v3 plane_from_pos_and_normal(v3 pos, v3 normal) {
  normal = v3_normalize_fast(normal);
  return (v3) {
    normal.x,
    normal.y,
    normal.z,
    -v3_dot(pos, normal)
  };
}

inline bool point_behind_plane(v3 pos, v3 plane) {
  return (pos.x * plane.x + pos.y * plane.y + pos.z * plane.z + plane.w) < 0.0f;
}

inline v3 project_to_screen(v3 p, i32 width, i32 height) {
  p.x += 1.0f;
  p.y += 1.0f;
  p.x *= 0.5f * width;
  p.y *= 0.5f * height;
  return p;
}

#ifdef USE_SSE

inline m4 transpose(m4 a) {
  m4 result = a;

  _MM_TRANSPOSE4_PS(result.rows[0], result.rows[1], result.rows[2], result.rows[3]);

  return result;
}

inline __m128 linear_combine(__m128 left, m4 right) {
  __m128 result;

  result = _mm_mul_ps(_mm_shuffle_ps(left, left, 0x00), right.rows[0]);
  result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0x55), right.rows[1]));
  result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0xaa), right.rows[2]));
  result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0xff), right.rows[3]));

  return result;
}

#endif
