#pragma once
#include "lea.h"
#include "rand_gen.h"
#include <stdint.h>

#define RAND_WORD (random_generator.rand_32bits())
#define RAND_BLOCK (block({RAND_WORD, RAND_WORD, RAND_WORD, RAND_WORD}))

void make_target_diff_samples(const uint32_t& n, const uint32_t& nr, const block& diff, block out0[], block out1[], const bool& positive_samples, const bool& calc_back=true, const uint32_t& version=128);
uint64_t extract_bits_from_block_9r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_10r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_11r(const block& t0, const block& t1);