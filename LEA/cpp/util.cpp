#include "util.h"
#include <stdint.h>
#include <stdio.h>

void make_target_diff_samples(const uint32_t& n, const uint32_t& nr, const block& diff, block out0[], block out1[], const bool& positive_samples, const bool& calc_back, const uint32_t& version) {
    word key[8];
    block p0, p1;
    word rk[MAX_NR][6];
    for (uint32_t i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) key[j] = RAND_WORD;
        expand_key(key, nr, rk, version);
        p0 = RAND_BLOCK;
        if (positive_samples) p1 = p0 ^ diff;
        else p1 = RAND_BLOCK;
        out0[i] = encrypt(p0, rk, nr);
        out1[i] = encrypt(p1, rk, nr);
        if (calc_back) {
            out0[i] = calculate_back(out0[i]);
            out1[i] = calculate_back(out1[i]);
        }
    }
}

// 33 selected bits
// dy0[30~28] || dy0[19~15] || dy0[10~9] || dy0[1~0] || dy1[30~28] || dy1[19~14] || dy1[1~0] || dy2[1~0] || dy3[1~0] || y1[29~28] || y1[18~15]
uint64_t extract_bits_from_block_9r(const block& t0, const block& t1) {
    uint64_t dy0 = t0.x0 ^ t1.x0, dy1 = t0.x1 ^ t1.x1, dy2 = t0.x2 ^ t1.x2, dy3 = t0.x3 ^ t1.x3;
    uint64_t y1 = t0.x1;
    uint64_t res = ((dy0 & 0x70000000u) << 2) | ((dy0 & 0xf8000u) << 10) | ((dy0 & 0x600u) << 14) | ((dy0 & 0x3u) << 21);
    res |= ((dy1 & 0x70000000u) >> 10) | ((dy1 & 0xfc000u) >> 2) | ((dy1 & 0x3u) << 10);
    res |= ((dy2 & 0x3u) << 8);
    res |= ((dy3 & 0x3u) << 6);
    res |= ((y1 & 0x30000000u) >> 24) | ((y1 & 0x78000u) >> 15);
    return res;
}

// 27 selecetd bits
// dy0[25~23] || dy0[14] || dy0[10~6] || dy1[25~22] || dy1[14~13] || dy1[10~6] || dy2[10] || dy2[7] || y1[24~22] || y1[9~8]
uint64_t extract_bits_from_block_10r(const block& t0, const block& t1) {
    uint64_t dy0 = t0.x0 ^ t1.x0, dy1 = t0.x1 ^ t1.x1, dy2 = t0.x2 ^ t1.x2;
    uint64_t y1 = t0.x1;
    uint64_t res = ((dy0 & 0x3800000u) << 1) | ((dy0 & 0x4000u) << 9) | ((dy0 & 0x7c0u) << 12);
    res |= ((dy1 & 0x3c00000u) >> 8) | ((dy1 & 0x6000u) >> 1) | ((dy1 & 0x7c0u) << 1);
    res |= ((dy2 & 0x400u) >> 4) | ((dy2 & 0x80u) >> 2);
    res |= ((y1 & 0x1c00000u) >> 20) | ((y1 & 0x300u) >> 8);
    return res;
}

// 17 selected bits
// dy0[19~18] || dy0[5~3] || dy1[19~18] || dy1[5~2] || y1[18~16] || y1[4~2]
uint64_t extract_bits_from_block_11r(const block& t0, const block& t1) {
    uint64_t dy0 = t0.x0 ^ t1.x0, dy1 = t0.x1 ^ t1.x1;
    uint64_t y1 = t0.x1;
    uint64_t res = ((dy0 & 0xc0000u) >> 3) | ((dy0 & 0x38u) << 9);
    res |= ((dy1 & 0xc0000u) >> 8) | ((dy1 & 0x3cu) << 4);
    res |= ((y1 & 0x70000u) >> 13) | ((y1 & 0x1cu) >> 2);
    return res;
}