// v3_dot.c

#include "common.h"
#include "maths.h"

#include "maths.c"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

#ifndef NO_OMP
  #include <omp.h>
#endif

i32 main(void) {
  random_init(time(0));

  size_t n    = 10000000;
  Arena a     = arena_new(sizeof(v3) * n + sizeof(f32) * n);
  v3* input   = arena_alloc(&a, sizeof(v3) * n);
  f32* output = arena_alloc(&a, sizeof(f32) * n);
  ASSERT(input && output);
  for (size_t i = 0; i < n; ++i) {
    input[i] = V3(
      random_f32() - random_f32(),
      random_f32() - random_f32(),
      random_f32() - random_f32()
    );
  }

  TIMER_START();

  #pragma omp parallel for
  for (size_t i = 0; i < n; ++i) {
    output[i] = v3_dot(input[i], input[i]);
  }
  f32 dt = TIMER_END();
  printf("%g ms\n", dt * 1000);
  arena_free(&a);
  return 0;
}
