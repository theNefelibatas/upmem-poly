#include <stdint.h>
#include <stdio.h>
#include <attributes.h>
#include <defs.h>
#include <mram.h>
#include <perfcounter.h>
#include "header.h"

#define BLOCK_SIZE (64)
#define CHUNK_SIZE ((POLY_SIZE + NR_DPUS - 1) / NR_DPUS)

__mram_noinit uint32_t POLYNOMIAL(1)[POLY_SIZE];
__mram_noinit uint32_t POLYNOMIAL(2)[POLY_SIZE];
__mram_noinit uint32_t POLYNOMIAL(0)[POLY_SIZE];

__dma_aligned uint32_t cache1[NR_TASKLETS][BLOCK_SIZE];
__dma_aligned uint32_t cache2[NR_TASKLETS][BLOCK_SIZE];
__dma_aligned uint32_t cacheo[NR_TASKLETS][BLOCK_SIZE];

__host op_t OP_TYPE;
__host dpu_args_t DPU_INPUT_ARGUMENTS;

typedef void(*op_p)(uint32_t, uint32_t);

void poly_addtion(uint32_t tid, uint32_t length);
void poly_multiplication(uint32_t tid, uint32_t length);

int main() {
    uint32_t length = DPU_INPUT_ARGUMENTS.length;
    uint32_t tid = me();

    op_p op = NULL;
    switch (OP_TYPE) {
    case ADD:
        op = poly_addtion;
        break;
    case MUL:
        op = poly_multiplication;
        break;
    default:
        op = poly_addtion;
    }

    if (tid == 0) {
        printf("NR_TASKLETS=%u, POLY_SIZE=%u, NR_DPUS=%u, CHUNK_SIZE=%u, length=%u\n",
               (unsigned)NR_TASKLETS, (unsigned)POLY_SIZE, (unsigned)NR_DPUS,
               (unsigned)CHUNK_SIZE, (unsigned)length);
    }
    for (uint32_t base = tid * BLOCK_SIZE; base < length; base += NR_TASKLETS * BLOCK_SIZE) {
        uint32_t n = BLOCK_SIZE;
        if (base + n > length) {
            n = length - base;
        }

        mram_read(&POLYNOMIAL(1)[base], cache1[tid], n * sizeof(uint32_t));
        mram_read(&POLYNOMIAL(2)[base], cache2[tid], n * sizeof(uint32_t));

        op(tid, n);

        mram_write(cacheo[tid], &POLYNOMIAL(0)[base], n * sizeof(uint32_t));
    }
    return 0;
}

void poly_addtion(uint32_t tid, uint32_t length) {
    for (uint32_t j = 0; j < length; j++) {
        uint32_t s = cache1[tid][j] + cache2[tid][j];
        if (s >= Q) {
            s -= Q;
        }
        cacheo[tid][j] = s;
    }
}

void poly_multiplication(uint32_t tid, uint32_t length) {
    for (uint32_t j = 0; j < length; j++) {
        uint64_t p = (uint64_t)(cache1[tid][j]) * cache2[tid][j];
        cacheo[tid][j] = p % Q;
    }
}
