#include "util.h"

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
uint64_t extract_bits_from_block_8r_ID1(const block& t0, const block& t1) {
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
uint64_t extract_bits_from_block_8r_ID2(const block& t0, const block& t1) {
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
uint64_t extract_bits_from_block_9r_ID2(const block& t0, const block& t1) {
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
uint64_t extract_bits_from_block_8r_ID3(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x70000ull) << 2) | ((res & 0x780ull) << 7);
    res |= ((dy_prime & 0x38000000ull) >> 16) | ((dy_prime & 0x3c0000ull) >> 11) | ((dy_prime & 0x3c00ull) >> 7);
    res |= (y0_prime & 0x1c0000ull) >> 18;
    return res;
}