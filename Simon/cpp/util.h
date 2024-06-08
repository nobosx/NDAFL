#ifndef UTIL_H
#define UTIL_H

#include "simon.h"

void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], const bool& whether_calc_back=true);
void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[], const bool& whether_calc_back=true);
void make_target_diff_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], const bool& Y, const bool& whether_calc_back=true);
void set_bit_val(uint64_t& x, const uint32_t& id, const uint8_t& val);
uint64_t extract_bits_from_block_simon32_9r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_simon32_10r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_simon32_11r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_simon64_12r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_simon64_13r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_simon128_19r(const block& t0, const block& t1);
#endif