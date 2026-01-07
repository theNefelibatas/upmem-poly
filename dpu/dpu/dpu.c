#include <stdint.h>
#include <stdio.h>
#include <attributes.h>
#include <defs.h>
#include <mram.h>
#include <perfcounter.h>
#include "header.h"

#define BLOCK_SIZE (64)

__mram_noinit uint32_t POLYNOMIAL(1)[POLY_SIZE];
__mram_noinit uint32_t POLYNOMIAL(2)[POLY_SIZE];
__mram_noinit uint32_t POLYNOMIAL(0)[POLY_SIZE];

__dma_aligned uint32_t cache1[NR_TASKLETS][BLOCK_SIZE];
__dma_aligned uint32_t cache2[NR_TASKLETS][BLOCK_SIZE];
__dma_aligned uint32_t cacheo[NR_TASKLETS][BLOCK_SIZE];

void poly_addtion();
void poly_multiplication();

int main() {
    poly_multiplication();
    return 0;
}

void poly_addtion() {
    uint32_t tid = me();

    if (tid == 0) {
        printf("NR_TASKLETS=%u, POLY_SIZE=%u\n", (unsigned)NR_TASKLETS, (unsigned)POLY_SIZE);
    }

    for (uint32_t base = tid * BLOCK_SIZE; base < POLY_SIZE; base += NR_TASKLETS * BLOCK_SIZE) {
        uint32_t n = BLOCK_SIZE;
        if (base + n > POLY_SIZE) {
            n = POLY_SIZE - base;
        }

        mram_read(&POLYNOMIAL(1)[base], cache1[tid], n * sizeof(uint32_t));
        mram_read(&POLYNOMIAL(2)[base], cache2[tid], n * sizeof(uint32_t));

        for (uint32_t j = 0; j < n; j++) {
            uint32_t s = cache1[tid][j] + cache2[tid][j];
            if (s >= Q) {
                s -= Q;
            }
            cacheo[tid][j] = s;
        }

        mram_write(cacheo[tid], &POLYNOMIAL(0)[base], n * sizeof(uint32_t));
    }
}

void poly_multiplication() {
    uint32_t tid = me();

    if (tid == 0) {
        printf("NR_TASKLETS=%u, POLY_SIZE=%u\n", (unsigned)NR_TASKLETS, (unsigned)POLY_SIZE);
    }

    for (uint32_t base = tid * BLOCK_SIZE; base < POLY_SIZE; base += NR_TASKLETS * BLOCK_SIZE) {
        uint32_t n = BLOCK_SIZE;
        if (base + n > POLY_SIZE) {
            n = POLY_SIZE - base;
        }

        mram_read(&POLYNOMIAL(1)[base], cache1[tid], n * sizeof(uint32_t));
        mram_read(&POLYNOMIAL(2)[base], cache2[tid], n * sizeof(uint32_t));

        for (uint32_t j = 0; j < n; j++) {
            uint64_t p = cache1[tid][j] * cache2[tid][j];
            cacheo[tid][j] = p % Q;
        }

        mram_write(cacheo[tid], &POLYNOMIAL(0)[base], n * sizeof(uint32_t));
    }
}
