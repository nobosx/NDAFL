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
// 25 selected bits
// dl[26~25] || dl[18~14] || dy[34~33] || dy[26~22] || dy[18~13] || y[17~13]
// dl[26~25] || dl[18~14] || dy'[37~36] || dy'[29~25] || dy'[21~16] || y'[20~16]
static uint64_t extract_bits_from_block_9r_ID1(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x6000000ull) >> 2) | ((res & 0x7c000ull) << 4);
    res |= ((dy_prime & 0x3000000000ull) >> 20) | ((dy_prime & 0x3e000000ull) >> 14) | ((dy_prime & 0x3f0000ull) >> 11);
    res |= (y0_prime & 0x1f0000ull) >> 16;
    return res;
}

// input diff = (0x80, 0x8000000000000000)
// 27 selected bits
// dl[29~28] || dl[21~16] || dy[37~36] || dy[29~24] || dy[21~16] || y[20~16]
// dl[29~28] || dl[21~16] || dy'[40~39] || dy'[32~27] || dy'[24~19] || y'[23~19]
static uint64_t extract_bits_from_block_9r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x30000000ull) >> 3) | ((res & 0x3f0000ull) << 3);
    res |= ((dy_prime & 0x18000000000ull) >> 22) | ((dy_prime & 0x1f8000000ull) >> 16) | ((dy_prime & 0x1f80000ull) >> 14);
    res |= (y0_prime & 0xf80000ull) >> 19;
    return res;
}

// input diff = (0x80, 0x8000000000000000)
// 26 selected bits
// dl[29~28] || dl[21~17] || dy[37~36] || dy[29~26] || dy[21~16] || y[28~27] || y[20~16]
// dl[29~28] || dl[21~17] || dy'[40~39] || dy'[32~29] || dy'[24~19] || y'[31~30] || y'[23~19]
static uint64_t extract_bits_from_block_10r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x30000000ull) >> 4) | ((res & 0x3e0000ull) << 2);
    res |= ((dy_prime & 0x18000000000ull) >> 22) | ((dy_prime & 0x1e0000000ull) >> 16) | ((dy_prime & 0x1f80000ull) >> 12);
    res |= ((y0_prime & 0xc0000000ull) >> 25) | ((y0_prime & 0xf80000ull) >> 19);
    return res;
}

// diff index = 64
// 18 selected bits
// dl[11~8] || dy[19~16] || dy[11~7] || y[10~6]
// dl[11~8] || dy'[22~19] || dy'[14~10] || y'[13~9]
//    17~14 ||     13~10  ||      9~5   ||     4~0
static uint64_t extract_bits_from_block_9r_diff64(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0xf00ull) << 6);
    res |= ((dy_prime & 0x780000ull) >> 9) | ((dy_prime & 0x7c00ull) >> 5);
    res |= ((y0_prime & 0x3e00ull) >> 9);
    return res;
}

// diff index = 76
// 20 selected bits
// dl[23~19] || dy[31~28] || dy[23~18] || y[22~18]
// dl[23~19] || dy'[34~31] || dy'[26~21] || y'[25~21]
//    19~15  ||     14~11  ||     10~5   ||     4~0
static uint64_t extract_bits_from_block_9r_diff76(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0xf80000ull) >> 4);
    res |= ((dy_prime & 0x780000000ull) >> 20) | ((dy_prime & 0x7e00000ull) >> 16);
    res |= ((y0_prime & 0x3e00000ull) >> 21);
    return res;
}

// diff index = 90
// 19 selected bits
// dl[37~33] || dy[45~42] || dy[37~33] || y[36~32]
// dl[37~33] || dy'[48~45] || dy'[40~36] || y'[39~35]
//    18~14  ||     13~10  ||      9~5   ||     4~0
static uint64_t extract_bits_from_block_9r_diff90(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x3e00000000ull) >> 19);
    res |= ((dy_prime & 0x1e00000000000ull) >> 35) | ((dy_prime & 0x1f000000000ull) >> 31);
    res |= ((y0_prime & 0xf800000000ull) >> 35);
    return res;
}

// diff index = 105
// 20 selected bits
// dl[52~48] || dy[60~57] || dy[52~47] || y[51~47]
// dl[52~48] || dy'[63~60] || dy'[55~50] || y'[54~50]
//    19~15  ||     14~11  ||     10~5   ||     4~0
static uint64_t extract_bits_from_block_9r_diff105(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1f000000000000ull) >> 33);
    res |= ((dy_prime & 0xf000000000000000ull) >> 49) | ((dy_prime & 0xfc000000000000ull) >> 45);
    res |= ((y0_prime & 0x7c000000000000ull) >> 50);
    return res;
}

// diff index = 113
// 24 selected bits
// dl[60~58] || dl[4~3] || dy[60~56] || dy[12~11] || dy[4~0] || y[59~56] || y[3~1]
// dl[60~58] || dl[4~3] || dy'[63~59] || dy'[15~14] || dy'[7~3] || y'[62~59] || y'[6~4]
//    23~21  ||   20~19 ||     18~14  ||     13~12  ||    11~7  ||     6~3   ||    2~0
static uint64_t extract_bits_from_block_9r_diff113(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x1c00000000000000ull) >> 37) | ((res & 0x18ull) << 16);
    res |= ((dy_prime & 0xf800000000000000ull) >> 45) | ((dy_prime & 0xc000ull) >> 2) | ((dy_prime & 0xf8ull) << 4);
    res |= ((y0_prime & 0x7800000000000000ull) >> 56) | ((y0_prime & 0x70ull) >> 4);
    return res;
}

InputExtracter get_extracter(const uint32_t& dis_setting) {
    InputExtracter result;
    switch(dis_setting) {
            break;
        case ID1_9R:
            result.inner_extracter = extract_bits_from_block_9r_ID1;
            break;
        case ID2_9R:
            result.inner_extracter = extract_bits_from_block_9r_ID2;
            break;
        case ID2_10R:
            result.inner_extracter = extract_bits_from_block_10r_ID2;
            break;
        case ATTACK_9R_DIFF64:
            result.inner_extracter = extract_bits_from_block_9r_diff64;
            break;
        case ATTACK_9R_DIFF76:
            result.inner_extracter = extract_bits_from_block_9r_diff76;
            break;
        case ATTACK_9R_DIFF90:
            result.inner_extracter = extract_bits_from_block_9r_diff90;
            break;
        case ATTACK_9R_DIFF105:
            result.inner_extracter = extract_bits_from_block_9r_diff105;
            break;
        case ATTACK_9R_DIFF113:
            result.inner_extracter = extract_bits_from_block_9r_diff113;
            break;
        default:
            printf("undefined distinguishing setting!!!\n");
            return result;
    }
    return result;
}

void generate_one_user_key(word user_key[M]) {
    for (int i = 0; i < M; i++) {
        user_key[i] = RAND_WORD;
    }
}

void generate_one_user_key(word user_key[M], RandomGenerator* random_generator) {
    for (int i = 0; i < M; i++) {
        user_key[i] = RAND_WORD_X(random_generator);
    }
}

block generate_one_plaintext() {
    return RAND_BLOCK;
}

block generate_one_plaintext(RandomGenerator* random_generator) {
    return RAND_BLOCK_X(random_generator);
}

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

void collect_ciphertext_structure(const uint32_t& n, const uint32_t& attack_nr, const block p0[], const block p1[], block c0[], block c1[], const word user_round_keys[]) {
    for (uint32_t i = 0; i < n; i++) {
        encrypt(p0[i], user_round_keys, attack_nr, c0[i]);
        encrypt(p1[i], user_round_keys, attack_nr, c1[i]);
    }
}