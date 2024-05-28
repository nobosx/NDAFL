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
// 29 selected bits
// dl[23~21] || dl[13] || dl[10~9] || dl[2~0] || dy[23~20] || dy[18~17] || dy[13~12] || dy[10~7] || dy[5] || dy[2~0] || y[21] || y[9~8] || y[1]
// dl[23~21] || dl[13] || dl[10~9] || dl[2~0] || dy'[23] || dy'[21~20] || dy'[16~15] || dy'[13~10] || dy'[8] || dy'[5~0] || y'[12~11] || y'[4] || y'[0]
uint64_t extract_bits_from_block_7r_ID1(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0xe00000u) << 5) | ((res & 0x2000u) << 12) | ((res & 0x600u) << 14) | ((res & 0x7u) << 20);
    res |= ((dy_prime & 0x800000u) >> 4) | ((dy_prime & 0x300000u) >> 3) | ((dy_prime & 0x18000u)) | ((dy_prime & 0x3c00u) << 1) | ((dy_prime & 0x100u) << 2) | ((dy_prime & 0x3fu) << 4);
    res |= ((y0_prime & 0x1800u) >> 9) | ((y0_prime & 0x10u) >> 3) | ((y0_prime & 0x1u));
    return res;
}

// input diff = (0x80, 0x800000)
// 29 selected bits
// dl[13~10] || dl[5~0] || dy[21~19] || dy[13~8] || dy[5~0] || y[12~9]
// dl[13~10] || dl[5~0] || dy'[23~22] || dy'[16~11] || dy'[8~3] || dy'[0] || y'[15~12]
uint64_t extract_bits_from_block_7r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x3c00u) << 15) | ((res & 0x3fu) << 19);
    res |= ((dy_prime & 0xc00000u) >> 5) | ((dy_prime & 0x1f800u)) | ((dy_prime & 0x1f8u) << 2) | ((dy_prime & 0x1u) << 4);
    res |= ((y0_prime & 0xf000u) >> 12);
    return res;
}