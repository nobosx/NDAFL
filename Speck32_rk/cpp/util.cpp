#include "util.h"
#include <vector>
#include <math.h>
#include <assert.h>
using namespace std;

void trans_lookup_table_output(const uint64_t lookup_table[], double transed_table[], const vector<uint32_t>& selected_bits) {
    uint64_t input_space = pow(2, selected_bits.size());
    uint64_t sample_num = 0;
    uint64_t average_num_log2;
    for (uint64_t i = 0; i < input_space; i++) sample_num += lookup_table[i];
    average_num_log2 = log2(sample_num / input_space);
    for (uint64_t i = 0; i < input_space; i++) transed_table[i] = log2(lookup_table[i]) - average_num_log2;
}

void make_encryption_data_rk(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], block c0[], block c1[], const uint32_t& key_repeat_num) {
    block p0, p1;
    uint32_t num_generated_data = 0;
    word mk[M], mk_prime[M];
    word rk[MAX_NR], rk_prime[MAX_NR];
    while (num_generated_data < n) {
        for (int i = 0; i < M; i++) {
            mk[i] = RAND_WORD;
            mk_prime[i] = mk[i] ^ mk_diff[i];
        }
        expand_key(mk, rk, num_rounds);
        expand_key(mk_prime, rk_prime, num_rounds);
        bool check_flag = true;
        // check whether the round keys conform to the given differential trail
        for (uint32_t i = 0; i < num_rounds; i++) {
            if ((rk[i] ^ rk_prime[i]) != rk_diff_trail[i]) {
                check_flag = false;
                break;
            }
        }
        if (!check_flag) continue;
        // reuse this key pair for key_repeat_num times
        for (uint32_t i = 0; i < key_repeat_num; i++) {
            if (num_generated_data == n) break;
            p0.first = RAND_WORD, p0.second = RAND_WORD;
            p1.first = p0.first ^ diff.first, p1.second = p0.second ^ diff.second;
            encrypt(p0, rk, num_rounds, c0[num_generated_data]);
            encrypt(p1, rk_prime, num_rounds, c1[num_generated_data]);
            ++num_generated_data;
        }
    }
}

void make_test_set_rk(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], block c0[], block c1[], bool Y[], const uint32_t& key_repeat_num) {
    block p0, p1;
    uint32_t num_generated_data = 0;
    word mk[M], mk_prime[M];
    word rk[MAX_NR], rk_prime[MAX_NR];
    while (num_generated_data < n) {
        for (int i = 0; i < M; i++) {
            mk[i] = RAND_WORD;
            mk_prime[i] = mk[i] ^ mk_diff[i];
        }
        expand_key(mk, rk, num_rounds);
        expand_key(mk_prime, rk_prime, num_rounds);
        bool check_flag = true;
        // check whether the round keys conform to the given differential trail
        for (uint32_t i = 0; i < num_rounds; i++) {
            if ((rk[i] ^ rk_prime[i]) != rk_diff_trail[i]) {
                check_flag = false;
                break;
            }
        }
        if (!check_flag) continue;
        // reuse this key pair for key_repeat_num times
        for (uint32_t i = 0; i < key_repeat_num; i++) {
            if (num_generated_data == n) break;
            p0.first = RAND_WORD, p0.second = RAND_WORD;
            bool tmp_Y = RAND_BYTE & 0x1;
            if (tmp_Y) p1.first = p0.first ^ diff.first, p1.second = p0.second ^ diff.second;
            else p1.first = RAND_WORD, p1.second = RAND_WORD;
            encrypt(p0, rk, num_rounds, c0[num_generated_data]);
            encrypt(p1, rk_prime, num_rounds, c1[num_generated_data]);
            Y[num_generated_data] = tmp_Y;
            ++num_generated_data;
        }
    }
}

// 27 selected bits:
// dl[12~8] || dl[5] || dl[3~0] || l[null] || dy[12~8] || dy[5] || dy[3~0] || y[11~8] || y[2~0]
// dl[12~8] || dl[5] || dl[3~0] || dy'[14~10] || dy'[7] || dy'[5~2] || y'[13~10] || y'[4~2]
uint64_t extract_bits_from_block_rk_9r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1f00) << 14) | ((res & 0x0020) << 16) | ((res & 0x000f) << 17);
    res |= ((dy_prime & 0x7c00) << 2) | ((dy_prime & 0x0080) << 4) | ((dy_prime & 0x003c) << 5);
    res |= ((y0_prime & 0x3c00) >> 7) | ((y0_prime & 0x001c) >> 2);
    return res;
}

// 21 selected bits:
// dl[12~10] || dl[5~2] || dy[12~9] || dy[5~1] || y[11~9] || y[2~1]
// dl[12~10] || dl[5~2] || dy'[14~11] || dy'[7~3] || y'[13~11] || y'[4~3]
uint64_t extract_bits_from_block_rk_10r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1c00) << 8) | ((res & 0x003c) << 12);
    res |= ((dy_prime & 0x7800) >> 1) | ((dy_prime & 0x00f8) << 2);
    res |= ((y0_prime & 0x3800) >> 9) | ((y0_prime & 0x0018) >> 3);
    return res;
}

// 29 selected bits:
// dl[12~8] || dl[5] || dl[3~0] || l[9] || dy[12~8] || dy[5] || dy[3~0] || y[11~7] || y[2~0]
// dl[12~8] || dl[5] || dl[3~0] || l[9] || dy'[14~10] || dy'[7] || dy'[5~2] || y'[13~9] || y'[4~2]
uint64_t extract_bits_from_block_rk_9r_ID3(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    uint64_t l0 = t0.first;
    res = ((res & 0x1f00) << 16) | ((res & 0x0020) << 18) | ((res & 0x000f) << 19);
    res |= (l0 & 0x0200) << 9;
    res |= ((dy_prime & 0x7c00) << 3) | ((dy_prime & 0x0080) << 5) | ((dy_prime & 0x003c) << 6);
    res |= ((y0_prime & 0x3e00) >> 6) | ((y0_prime & 0x001c) >> 2);
    return res;
}

// 21 selected bits:
// dl[12~10] || dl[5~2] || dy[12~9] || dy[5~1] || y[11~9] || y[2~1]
// dl[12~10] || dl[5~2] || dy'[14~11] || dy'[7~3] || y'[13~11] || y'[4~3]
uint64_t extract_bits_from_block_rk_10r_ID3(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    uint64_t l0 = t0.first;
    res = ((res & 0x1c00) << 8) | ((res & 0x003c) << 12);
    res |= ((dy_prime & 0x7800) >> 1) | ((dy_prime & 0x00f8) << 2);
    res |= ((y0_prime & 0x3800) >> 9) | ((y0_prime & 0x0018) >> 3);
    return res;
}