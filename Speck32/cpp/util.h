#pragma once
#include "speck.h"
#include <vector>
using namespace std;

void cal_mean_std(const double data[], const uint32_t& length, double& mu, double& sigma);
uint64_t extract_bits_from_block_8r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_7r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_6r(const block& t0, const block& t1);
uint64_t extract_bits_from_block_5r(const block& t0, const block& t1);
void make_test_set(const uint32_t& n, const block& diff, const uint32_t& num_rounds, block c0[], block c1[], bool Y[]);
void make_target_diff_samples(const uint32_t& n, const block& diff, const uint32_t& num_rounds, block c0[], block c1[], bool whether_positive=true);
bool make_multi_input_sample(const uint32_t& structure_size, const block& diff, const uint32_t& num_rounds, block c0[], block c1[]);
void trans_lookup_table_output(const uint64_t lookup_table[], double transed_table[], const vector<uint32_t>& selected_bits);
uint32_t cal_hw(uint64_t input, const uint32_t& length=16);
uint32_t gen_rand_uint32_with_linear_constraint(const vector<linear_constraint>& constraints);
void expand_plaintext_structure(const block& diff, const vector<neutral_bit>& neutral_bits, block p0[], block p1[]);
void gen_extended_plaintext_structure(block p0[], block p1[], block paired_p0[], block paired_p1[], const block& diff1, const block& diff2, const vector<neutral_bit>& NBs, const uint32_t& switch_bit, const uint32_t& paired_bit, const vector<linear_constraint>& plaintext_conditions);
void collect_ciper_structure(const block p0[], const block p1[], const word& guessed_k0_bits, word ks[], const uint32_t& structure_size, const uint32_t& nr, block c0[], block c1[]);