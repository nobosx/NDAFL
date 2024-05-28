#include <math.h>
#include <assert.h>
#include <stdint.h>
#include "speck.h"
using namespace std;

void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[]) {
     block p0, p1;
    word mk[M], rk[MAX_NR];

    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        p1.first = p0.first ^ diff.first;
        p1.second = p0.second ^ diff.second;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
    }
}

void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[]) {
    block p0, p1;
    word mk[M], rk[MAX_NR];
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

// input diff = (0x80, 0x80000000)
// 18 selecetd bits
// dl[13~10] || dl[3] || dy[21~19] || dy[13~9] || dy[3] || y[12~9]
// dl[13~10] || dl[3] || dy'[24~22] || dy'[16~12] || dy'[6] || y'[15~12]
uint64_t extract_bits_from_block_8r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x00003c00u) << 4) | ((res & 0x00000008u) << 10);
    res |= ((dy_prime & 0x01c00000u) >> 12) | ((dy_prime & 0x0001f000u) >> 7) | ((dy_prime & 0x00000040u) >> 2);
    res |= ((y0_prime & 0x0000f000) >> 12);
    return res;
}

// input diff = (0x80, 0x80000000)
// 24 selected bits
// dl[21~20] || dl[13~9] || dy[29~28] || dy[21~18] || dy[13~8] || y[12~8]
// dl[21~20] || dl[13~9] || dy'[31] || dy'[24~21] || dy'[16~11] || dy'[0] || y'[15~11]
uint64_t extract_bits_from_block_7r_ID2(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0x00300000u) << 2) | ((res & 0x00003e00u) << 8);
    res |= ((dy_prime & 0x80000000u) >> 15) | ((dy_prime & 0x01e00000u) >> 9) | ((dy_prime & 0x0001f800u) >> 5) | ((dy_prime & 0x00000001u) << 5);
    res |= ((y0_prime & 0x0000f800u) >> 11);
    return res;
}

// input diff = (0x80, 0)
// 28 selected bits
// dl[31~29] || dl[21] || dl[10~7] || dl[0] || dy[31~28] || dy[21] || dy[18~16] || dy[10~5] || dy[0] || y[9~6]
// dl[31~29] || dl[21] || dl[10~7] || dl[0] || dy'[31] || dy'[24] || dy'[21~19] || dy'[13~8] || dy'[3~0] || y'[12~9]
uint64_t extract_bits_from_block_7r_ID1(const block& t0, const block& t1) {
    uint64_t y0_prime = t0.first ^ t0.second, y1_prime = t1.first ^ t1.second;
    uint64_t dy_prime = y0_prime ^ y1_prime;
    uint64_t res = t0.first ^ t1.first;
    res = ((res & 0xe0000000u) >> 4) | ((res & 0x00200000u) << 3) | ((res & 0x00000780u) << 13) | ((res & 0x1u) << 19);
    res |= ((dy_prime & 0x80000000u) >> 13) | ((dy_prime & 0x01000000u) >> 7) | ((dy_prime & 0x00380000u) >> 5) | ((dy_prime & 0x00003f00u)) | ((dy_prime & 0x0000000fu) << 4);
    res |= (y0_prime & 0x00001e00u) >> 9;
    return res;
}