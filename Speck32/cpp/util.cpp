#include "util.h"
#include <vector>
#include <math.h>
using namespace std;

void cal_mean_std(const double data[], const uint32_t& length, double& mu, double& sigma) {
    double sum = 0;
    for (uint32_t i = 0; i < length; i++) sum += data[i];
    mu = sum / length;
    sum = 0;
    for (uint32_t i = 0; i < length; i++) {
        sum += pow(data[i] - mu, 2);
    }
    sigma = sqrt(sum / length);
}

uint64_t extract_bits_from_block_8r(const block& t0, const block& t1) {
    // 22 selected bits
    // dl[12~10] || dl[5~1] || dy[12~10] || dy[5~1] || y0[11~10] || y0[4~1]
    // dl[12~10] || dl[5~1] || dy'[14~12] || dy'[7~3] || y0'[13~12] || y0'[6~3]
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1c00) << 9) | ((res & 0x003e) << 13);
    res |= ((dy_prime & 0x7000) >> 1) | ((dy_prime & 0x00f8) << 3);
    res |= ((y0_prime & 0x3000) >> 8) | ((y0_prime & 0x0078) >> 3);
    return res;
}

uint64_t extract_bits_from_block_7r(const block& t0, const block& t1) {
    // 24 selected bits
    // dl[12~9] || dl[5~2] || dy[12~8] || dy[5~1] || y[11~8] || y[2~1]
    // dl[12~9] || dl[5~2] || dy'[14~10] || dy'[7~3] || y'[13~10] || y'[4~3]
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1e00) << 11) | ((res & 0x003c) << 14);
    res |= ((dy_prime & 0x7c00) << 1) | ((dy_prime & 0x00f8) << 3);
    res |= ((y0_prime & 0x3c00) >> 8) | ((y0_prime & 0x0018) >> 3);
    return res;
}

uint64_t extract_bits_from_block_6r(const block& t0, const block& t1) {
    // 25 selected bits
    // dl[12~8] || dl[3~0] || dy[12~8] || dy[3~0] || y[11~8] || y[2~0]
    // dl[12~8] || dl[3~0] || dy'[14~10] || dy'[5~2] || y'[13~10] || y'[4~2]
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1f00) << 12) | ((res & 0xf) << 16);
    res |= ((dy_prime & 0x7c00) << 1) | ((dy_prime & 0x3c) << 5);
    res |= ((y0_prime & 0x3c00) >> 7) | ((y0_prime & 0x1c) >> 2);
    return res;
}

uint64_t extract_bits_from_block_5r(const block& t0, const block& t1) {
    // 26 selected bits
    // dl[12~8] || dl[3~0] || dy[12~7] || dy[3~0] || y[11~8] || y[2~0]
    // dl[12~8] || dl[3~0] || dy'[14~9] || dy'[5~2] || y'[13~10] || y'[4~2]
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1f00) << 13) | ((res & 0xf) << 17);
    res |= ((dy_prime & 0x7e00) << 2) | ((dy_prime & 0x3c) << 5);
    res |= ((y0_prime & 0x3c00) >> 7) | ((y0_prime & 0x1c) >> 2);
    return res;
}


void make_test_set(const uint32_t& n, const block& diff, const uint32_t& num_rounds, block c0[], block c1[], bool Y[]) {
    word ks[50];
    word mk[M];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, ks, num_rounds);
        p0 = {RAND_WORD, RAND_WORD};
        Y[i] = RAND_BYTE & 1;
        if (Y[i]) {
            p1.first = p0.first ^ diff.first;
            p1.second = p0.second ^ diff.second;
        } else {
            p1 = {RAND_WORD, RAND_WORD};
        }
        encrypt(p0, ks, num_rounds, c0[i]);
        encrypt(p1, ks, num_rounds, c1[i]);
    }
}

void make_target_diff_samples(const uint32_t& n, const block& diff, const uint32_t& num_rounds, block c0[], block c1[], bool whether_positive) {
    word ks[50];
    word mk[M];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, ks, num_rounds);
        p0 = {RAND_WORD, RAND_WORD};
        if (whether_positive) {
            p1.first = p0.first ^ diff.first;
            p1.second = p0.second ^ diff.second;
        } else {
            p1 = {RAND_WORD, RAND_WORD};
        }
        encrypt(p0, ks, num_rounds, c0[i]);
        encrypt(p1, ks, num_rounds, c1[i]);
    }
}

bool make_multi_input_sample(const uint32_t& structure_size, const block& diff, const uint32_t& num_rounds, block c0[], block c1[]) {
    word ks[50];
    word mk[M];
    block p0, p1;
    bool Y = RAND_BYTE & 1;
    for (uint32_t i = 0; i < structure_size; i++) {
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, ks, num_rounds);
        p0 = {RAND_WORD, RAND_WORD};
        if (Y) {
            p1.first = p0.first ^ diff.first;
            p1.second = p0.second ^ diff.second;
        } else {
            p1 = {RAND_WORD, RAND_WORD};
        }
        encrypt(p0, ks, num_rounds, c0[i]);
        encrypt(p1, ks, num_rounds, c1[i]);
    }
    return Y;
}

// convert each counter entry to the corresponding logarithm of real-vs-random likelihood ratio
void trans_lookup_table_output(const uint64_t lookup_table[], double transed_table[], const vector<uint32_t>& selected_bits) {
    uint64_t input_space = pow(2, selected_bits.size());
    uint64_t sample_num = 0;
    uint64_t average_num_log2;
    for (uint64_t i = 0; i < input_space; i++) sample_num += lookup_table[i];
    average_num_log2 = log2(sample_num / input_space);
    for (uint64_t i = 0; i < input_space; i++) transed_table[i] = log2(lookup_table[i]) - average_num_log2;
}

uint32_t cal_hw(uint64_t input, const uint32_t& length) {
    uint32_t res = 0;
    for (int i = 0; i < length; i++) {
        res += input & 1;
        input >>= 1;
    }
    return res;
}

uint32_t gen_rand_uint32_with_linear_constraint(const vector<linear_constraint>& constraints) {
    uint32_t rand_int = RAND_WORD;
    rand_int = (rand_int << 16) | RAND_WORD;
    uint32_t tmp;
    for (auto x : constraints) {
        tmp = x.xor_value;
        for (int i = 0; i < x.num_bits; i++) tmp ^= (rand_int >> x.xor_bit_pos[i]);
        rand_int ^= (tmp & 0x1) << x.xor_bit_pos[0];
    }
    return rand_int;
}

void expand_plaintext_structure(const block& diff, const vector<neutral_bit>& neutral_bits, block p0[], block p1[]) {
    uint32_t structure_size = 1;
    for (auto x : neutral_bits) {
        word dl = 0, dr = 0;
        for (int i = 0; i < x.bit_size; i++) {
            if (x.bit_pos[i] < WORD_SIZE) {
                dr |= 1 << x.bit_pos[i];
            } else {
                dl |= 1 << (x.bit_pos[i] - WORD_SIZE);
            }
        }
        for (int i = 0, j = structure_size; i < structure_size; i++, j++) {
            p0[j].first = p0[i].first ^ dl;
            p0[j].second = p0[i].second ^ dr;
        }
        structure_size <<= 1;
    }
    for (int i = 0; i < structure_size; i++) {
        p1[i].first = p0[i].first ^ diff.first;
        p1[i].second = p0[i].second ^ diff.second;
    }
}

// generate a plaintext structure containing 2^12 plaintext pairs for the 13-round attack
void gen_extended_plaintext_structure(block p0[], block p1[], block paired_p0[], block paired_p1[], const block& diff1, const block& diff2, const vector<neutral_bit>& NBs, const uint32_t& switch_bit, const uint32_t& paired_bit, const vector<linear_constraint>& plaintext_conditions) {
    uint32_t rand_int = gen_rand_uint32_with_linear_constraint(plaintext_conditions);
    p0[0].first = rand_int >> 16, p0[0].second = rand_int & 0xffff;
    uint32_t structure_size = 1 << NBs.size();
    expand_plaintext_structure(diff1, NBs, p0, p1);
    word dl = 0, dr = 0;
    if (switch_bit < WORD_SIZE) dr |= 1 << switch_bit;
    else dl |= 1 << (switch_bit - WORD_SIZE);
    for (int i = 0, j = structure_size; i < structure_size; i++, j++) {
        p0[j].first = p0[i].first ^ dl;
        p0[j].second = p0[i].second ^ dr;
        p1[j].first = p0[j].first ^ diff2.first;
        p1[j].second = p0[j].second ^ diff2.second;
    }
    structure_size <<= 1;
    dl = 0, dr = 0;
    if (paired_bit < WORD_SIZE) dr |= 1 << paired_bit;
    else dl |= 1 << (paired_bit - WORD_SIZE);
    for (int i = 0, j = structure_size; i < structure_size; i++, j++) {
        p0[j].first = p0[i].first ^ dl;
        p0[j].second = p0[i].second ^ dr;
        p1[j].first = p1[i].first ^ dl;
        p1[j].second = p1[i].second ^ dr;
    }
    for (int i = 0, j = structure_size; i < structure_size; i++, j++) {
        paired_p0[i] = p0[i];
        paired_p1[i] = p1[j];
        paired_p0[j] = p0[j];
        paired_p1[j] = p1[i];
    }
}

void collect_ciper_structure(const block p0[], const block p1[], const word& guessed_k0_bits, word ks[], const uint32_t& structure_size, const uint32_t& nr, block c0[], block c1[]) {
    block t0, t1;
    for (uint32_t i = 0; i < structure_size; i++) {
        dec_one_round(p0[i], guessed_k0_bits, t0);
        dec_one_round(p1[i], guessed_k0_bits, t1);
        encrypt(t0, ks, nr, c0[i]);
        encrypt(t1, ks, nr, c1[i]);
    }
}