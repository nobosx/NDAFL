#ifndef UTIL_H
#define UTIL_H

#include "speck.h"
#include <stdint.h>
#include <vector>
using namespace std;

#define ID1_9R 1
#define ID2_9R 2
#define ID2_10R 3
#define ATTACK_9R_DIFF64 4
#define ATTACK_9R_DIFF76 5
#define ATTACK_9R_DIFF90 6
#define ATTACK_9R_DIFF105 7
#define ATTACK_9R_DIFF113 8

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
void generate_one_user_key(word user_key[M], RandomGenerator* random_generator);
// 随机产生一个明文
block generate_one_plaintext();
block generate_one_plaintext(RandomGenerator* random_generator);
void generate_one_plaintext_structure(const block& diff, block p0[], block p1[], const vector<uint32_t>& NBs);
void collect_ciphertext_structure(const uint32_t& n, const uint32_t& attack_nr, const block p0[], const block p1[], block c0[], block c1[], const word user_round_keys[]);
#endif