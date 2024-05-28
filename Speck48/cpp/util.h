#ifndef UTIL_H
#define UTIL_H

#include "speck.h"
#include <stdint.h>

#define ID1_7R 1
#define ID2_7R 2

void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[]);
void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[]);
uint64_t extract_bits_from_block_7r_ID1(const block& t0, const block& t1);
uint64_t extract_bits_from_block_7r_ID2(const block& t0, const block& t1);
#endif