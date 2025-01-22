#ifndef UTIL_H
#define UTIL_H

#include "speck.h"
#include <stdint.h>
#include <vector>
using namespace std;

#define ID1_8R 1
#define ID2_8R 2
#define ID2_9R 3
#define ID3_8R 4
#define ATTACK_8R_21_20_AND_13_8 5
#define ATTACK_8R_29_8 6
#define ATTACK_7R_29_16 7
#define ATTACK_7R_29_8 8

class InputExtracter{
    public:
    uint64_t (*inner_extracter)(const block&, const block&);
    uint64_t extract_distinguisher_input(const block& t0, const block& t1) const;
};

uint32_t cal_hw(uint64_t x, const int& length);
InputExtracter get_extracter(const uint32_t& dis_setting);
void build_response_table(const uint32_t& input_bits, const uint64_t lookup_table[], double response_table[]);
void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[]);
void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[]);
void generate_one_user_key(word user_key[M]);
void generate_one_user_key(word user_key[M], RandomGenerator* r);
block generate_one_plaintext();
block generate_one_plaintext(RandomGenerator* r);
void generate_one_plaintext_structure(const block& diff, block p0[], block p1[], const vector<uint32_t>& NBs);
#endif