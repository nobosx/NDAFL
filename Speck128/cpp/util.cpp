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
// 25 selected bits
// dl[26~25] || dl[18~14] || dy[34~33] || dy[26~22] || dy[18~13] || y[17~13]
// dl[26~25] || dl[18~14] || dy'[37~36] || dy'[29~25] || dy'[21~16] || y'[20~16]
uint64_t extract_bits_from_block_9r_ID1(const block& t0, const block& t1) {
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
uint64_t extract_bits_from_block_9r_ID2(const block& t0, const block& t1) {
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
uint64_t extract_bits_from_block_10r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x30000000ull) >> 4) | ((res & 0x3e0000ull) << 2);
    res |= ((dy_prime & 0x18000000000ull) >> 22) | ((dy_prime & 0x1e0000000ull) >> 16) | ((dy_prime & 0x1f80000ull) >> 12);
    res |= ((y0_prime & 0xc0000000ull) >> 25) | ((y0_prime & 0xf80000ull) >> 19);
    return res;
}