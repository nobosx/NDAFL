#include "util.h"
#include <stdio.h>
#include <math.h>

uint64_t InputExtracter::extract_distinguisher_input(const block& t0, const block& t1) const {
    return (*inner_extracter)(t0, t1);
}

uint32_t cal_hw(uint64_t x, const int& length) {
    uint32_t res = 0;
    for (int i = 0; i < length; i++) {
        res += x & 1;
        x >>= 1;
    }
    return res;
}

void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[]) {
    word mk[M], rk[MAX_NR];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        p1.first = p0.first ^ diff.first; p1.second = p0.second ^ diff.second;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
    }
}

void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[]) {
    word mk[M], rk[MAX_NR];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        bool tmp_Y = RAND_BYTE & 0x1;
        if (tmp_Y) p1.first = p0.first ^ diff.first, p1.second = p0.second ^ diff.second;
        else p1 = RAND_BLOCK;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
        Y[i] = tmp_Y;
    }
}

// input diff = (0x80, 0)
// 26 selected bits
// dl[18~16] || dl[10~8] || dl[1~0] || dy[26~24] || dy[18~15] || dy[10~8] || dy[1~0] || y[17~15] || y[9~7]
// dl[18~16] || dl[10~8] || dl[1~0] || dy'[29~27] || dy'[21~18] || dy'[13~11] || dy'[4~3] || y'[20~18] || y'[12~10]
static uint64_t extract_bits_from_block_8r_ID1(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x70000ull) << 7) | ((res & 0x700ull) << 12) | ((res & 0x3ull) << 18);
    res |= ((dy_prime & 0x38000000ull) >> 12) | ((dy_prime & 0x3c0000ull) >> 7) | ((dy_prime & 0x3800ull) >> 3) | ((dy_prime & 0x18ull) << 3);
    res |= ((y0_prime & 0x1c0000ull) >> 15) | ((y0_prime & 0x1c00ull) >> 10);
    return res;
}

// input diff = (0x80, 0x800000000000)
// 25 selected bits
// dl[29] || dl[21~17] || dl[13~12] || dy[37] || dy[29~25] || dy[21~17] || dy[13~12] || y[20~17]
// dl[29] || dl[21~17] || dl[13~12] || dy'[40] || dy'[32~28] || dy[24~20] || dy'[16~15] || y'[23~20]
static uint64_t extract_bits_from_block_8r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x20000000ull) >> 5) | ((res & 0x3e0000ull) << 2) | ((res & 0x3000ull) << 5);
    res |= ((dy_prime & 0x10000000000ull) >> 24) | ((dy_prime & 0x1f0000000ull) >> 17) | ((dy_prime & 0x1f00000ull) >> 14) | ((dy_prime & 0x18000ull) >> 11);
    res |= (y0_prime & 0xf00000ull) >> 20;
    return res;
}

// input diff = (0x80, 0x800000000000)
// 22 selected bits
// dl[21~19] || dl[13~9] || dy[29~27] || dy[21~19] || dy[13~9] || y[20~18]
// dl[21~19] || dl[13~9] || dy'[32~30] || dy'[24~22] || dy'[16~12] || y'[23~21]
static uint64_t extract_bits_from_block_9r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = (res & 0x380000ull) | ((res & 0x3e00ull) << 5);
    res |= ((dy_prime & 0x1c0000000ull) >> 19) | ((dy_prime & 0x1c00000ull) >> 14) | ((dy_prime & 0x1f000ull) >> 9);
    res |= ((y0_prime & 0xe00000ull) >> 21);
    return res;
}

// input diff = (0, 0x800000000000)
// 21 selected bits
// dl[18~16] || dl[10~7] || dy[26~24] || dy[18~15] || dy[10~7] || y[17~15]
// dl[18~16] || dl[10~7] || dy'[29~27] || dy'[21~18] || dy'[13~10] || y'[20~18]
static uint64_t extract_bits_from_block_8r_ID3(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x70000ull) << 2) | ((res & 0x780ull) << 7);
    res |= ((dy_prime & 0x38000000ull) >> 16) | ((dy_prime & 0x3c0000ull) >> 11) | ((dy_prime & 0x3c00ull) >> 7);
    res |= (y0_prime & 0x1c0000ull) >> 18;
    return res;
}

// 14 selected bits
// dl[10~8] || dy[18~17] || dy[10~6] || y[9~6]
// dl[10~8] || dy'[21~20] || dy'[13~9] || y'[12~9]
//    13~11 ||     10~9   ||      8~4  ||     3~0
static uint64_t extract_bits_from_block_8r_21_20_and_13_8(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x700) << 3);
    res |= ((dy_prime & 0x300000) >> 11) | ((dy_prime & 0x3e00) >> 5);
    res |= ((y0_prime & 0x1e00) >> 9);
    return res;
}

// 23 selected bits
// dl[18~15] || dl[10~8] || dy[26~24] || dy[18~15] || dy[10~8] || y[17~15] || y[9~7]
// dl[18~15] || dl[10~8] || dy'[29~27] || dy'[21~18] || dy'[13~11] || y'[20~18] || y'[12~10]
//    22~19  ||    18~16 ||     15~13  ||     12~9   ||      8~6   ||     5~3   ||     2~0     
static uint64_t extract_bits_from_block_8r_29_8(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x78000) << 4) | ((res & 0x700) << 8);
    res |= ((dy_prime & 0x38000000) >> 14) | ((dy_prime & 0x3c0000) >> 9) | ((dy_prime & 0x3800) >> 5);
    res |= ((y0_prime & 0x1c0000) >> 15) | ((y0_prime & 0x1c00) >> 10);
    return res;
}

// 16 selected bits
// dl[18~16] || dy[26~22] || dy[18~14] || y[17~15]
// dl[18~16] || dy'[29~25] || dy'[21~17] || y'[20~18]
//    15~13  ||     12~8   ||      7~3   ||     2~0
static uint64_t extract_bits_from_block_7r_29_16(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x70000) >> 3);
    res |= ((dy_prime & 0x3e000000) >> 17) | ((dy_prime & 0x3e0000) >> 14);
    res |= ((y0_prime & 0x1c0000) >> 18);
    return res;
}

// 26 selected bits
// dl[18~15] || dl[10~8] || dy[26~23] || dy[18~14] || dy[10~8] || y[17~14] || y[9~7]
// dl[18~15] || dl[10~8] || dy'[29~26] || dy'[21~17] || dy'[13~11] || y'[20~17] || y'[12~10]
//    25~22  ||    21~19 ||     18~15  ||     14~10  ||      9~7   ||     6~3   ||     2~0
static uint64_t extract_bits_from_block_7r_29_8(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x78000) << 7) | ((res & 0x700) << 11);
    res |= ((dy_prime & 0x3c000000) >> 11) | ((dy_prime & 0x3e0000) >> 7) | ((dy_prime & 0x3800) >> 4);
    res |= ((y0_prime & 0x1e0000) >> 14) | ((y0_prime & 0x1c00) >> 10);
    return res;
}

InputExtracter get_extracter(const uint32_t& dis_setting) {
    InputExtracter result;
    switch(dis_setting) {
        case ID1_8R:
            result.inner_extracter = extract_bits_from_block_8r_ID1;
            break;
        case ID2_8R:
            result.inner_extracter = extract_bits_from_block_8r_ID2;
            break;
        case ID2_9R:
            result.inner_extracter = extract_bits_from_block_9r_ID2;
            break;
        case ID3_8R:
            result.inner_extracter = extract_bits_from_block_8r_ID3;
            break;
        case ATTACK_8R_21_20_AND_13_8:
            result.inner_extracter = extract_bits_from_block_8r_21_20_and_13_8;
            break;
        case ATTACK_8R_29_8:
            result.inner_extracter = extract_bits_from_block_8r_29_8;
            break;
        case ATTACK_7R_29_16:
            result.inner_extracter = extract_bits_from_block_7r_29_16;
            break;
        case ATTACK_7R_29_8:
            result.inner_extracter = extract_bits_from_block_7r_29_8;
            break;
        default:
            printf("undefined distinguishing setting!!!\n");
            exit(0);
    }
    return result;
}

void build_response_table(const uint32_t& input_bits, const uint64_t lookup_table[], double response_table[]) {
    uint64_t input_space = 1ull << input_bits;
    uint64_t sample_num = 0;
    uint64_t average_num;
    for (uint64_t i = 0; i < input_space; i++)
        sample_num += lookup_table[i];
    average_num = sample_num / input_space;
    printf("average_num is %d.\n", average_num);
    double average_num_log2 = log2(average_num);
    for (uint64_t i = 0; i < input_space; i++)
        if (lookup_table[i] == 0) {
            response_table[i] = -average_num_log2;
        } else {
            response_table[i] = log2(lookup_table[i]) - average_num_log2;
        }
}

void generate_one_user_key(word user_key[M]) {
    for (int i = 0; i < M; i++) {
        user_key[i] = RAND_WORD;
    }
}

void generate_one_user_key(word user_key[M], RandomGenerator* r) {
    for (int i = 0; i < M; i++) {
        user_key[i] = RAND_WORD_X(r);
    }
}

block generate_one_plaintext() {
    return RAND_BLOCK;
}

block generate_one_plaintext(RandomGenerator* r) {
    return RAND_BLOCK_X(r);
}

// Expand one plaintext p0[0] to a plaintext structure using neutral bits and a plaintext difference
void generate_one_plaintext_structure(const block& diff, block p0[], block p1[], const vector<uint32_t>& NBs) {
    uint32_t structure_size = 1;
    for (auto nb : NBs) {
        word diff_l = 0, diff_r = 0;
        if (nb < WORD_SIZE)
            diff_r = 1ull << nb;
        else
            diff_l = 1ull << (nb - WORD_SIZE);
        for (uint32_t i = 0; i < structure_size; i++) {
            p0[i + structure_size].first = p0[i].first ^ diff_l;
            p0[i + structure_size].second = p0[i].second ^ diff_r;
        }
        structure_size <<= 1;
    }
    for (uint32_t i = 0; i < structure_size; i++) {
        p1[i].first = p0[i].first ^ diff.first;
        p1[i].second = p0[i].second ^ diff.second;
        dec_one_round(p0[i], 0, p0[i]);
        dec_one_round(p1[i], 0, p1[i]);
    }
}