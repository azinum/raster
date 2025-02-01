// matmul.c

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
  Arena a     = arena_new(sizeof(m4) * n * 2);
  m4* input   = arena_alloc_t(m4, &a, n);
  m4* output  = arena_alloc_t(m4, &a, n);
  ASSERT(input && output);
  for (size_t i = 0; i < n; ++i) {
    for (size_t col = 0; col < 4; ++col) {
      for (size_t row = 0; row < 4; ++row) {
        input[i].e[col][row] = random_f32() - random_f32();
      }
    }
  }

  TIMER_START();

  #pragma omp parallel for
  for (size_t i = 0; i < n; ++i) {
    output[i] = m4_multiply(input[i], input[i]);
  }
  f32 dt = TIMER_END();
  printf("%g ms\n", dt * 1000);
  arena_free(&a);
  return 0;
}
