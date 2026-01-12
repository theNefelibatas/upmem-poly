#pragma once

#include <stdint.h>

#define POLYNOMIAL(NUM) polynomial_##NUM

#define POLY_SIZE (65536)
#define Q (65536 + 1)

typedef struct dpu_args_t {
    uint32_t length;
} dpu_args_t;
