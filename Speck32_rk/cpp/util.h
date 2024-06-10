#pragma once
#include "speck.h"
#include <vector>
using namespace std;

#define ID2_9R 1
#define ID2_10R 2
#define ID3_9R 3
#define ID3_10R 4

uint64_t extract_bits_from_block_rk_9r_ID2(const block& t0, const block& t1);
uint64_t extract_bits_from_block_rk_10r_ID2(const block& t0, const block& t1);
uint64_t extract_bits_from_block_rk_9r_ID3(const block& t0, const block& t1);
uint64_t extract_bits_from_block_rk_10r_ID3(const block& t0, const block& t1);
void trans_lookup_table_output(const uint64_t lookup_table[], double transed_table[], const vector<uint32_t>& selected_bits);
void make_encryption_data_rk(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], block c0[], block c1[], const uint32_t& key_repeat_num=(1<<10));
void make_test_set_rk(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], block c0[], block c1[], bool Y[], const uint32_t& key_repeat_num=(1<<10));