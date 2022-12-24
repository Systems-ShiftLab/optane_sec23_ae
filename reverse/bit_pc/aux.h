#pragma once
#include <stdint.h>

unsigned int count_bits(uint64_t n);
void gen_bitmask_perm(uint64_t *output, uint64_t count, uint64_t bitmask);
