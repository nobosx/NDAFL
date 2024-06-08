#include "util.h"

void set_bit_val(uint64_t& x, const uint32_t& id, const uint8_t& val) {
    if (((x >> id) & 1) != val) x ^= 1 << id;
}

void make_encryption_data(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], const bool& whether_calc_back) {
    word mk[M];
    word rk[MAX_NR];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        p1.first = p0.first ^ diff.first; p1.second = p0.second ^ diff.second;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
    }
    if (whether_calc_back) {
        for (uint32_t i = 0; i < n; i++) {
            c0[i] = calc_back(c0[i]);
            c1[i] = calc_back(c1[i]);
        }
    }
}

void make_test_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], bool Y[], const bool& whether_calc_back) {
    word mk[M];
    word rk[MAX_NR];
    block p0, p1;
    bool tmp_Y;
    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        tmp_Y = RAND_BYTE & 0x1;
        if (tmp_Y) p1.first = p0.first ^ diff.first, p1.second = p0.second ^ diff.second;
        else p1 = RAND_BLOCK;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
        Y[i] = tmp_Y;
    }
    if (whether_calc_back) {
        for (uint32_t i = 0; i < n; i++) {
            c0[i] = calc_back(c0[i]);
            c1[i] = calc_back(c1[i]);
        }
    }
}

void make_target_diff_set(const uint32_t& n, const uint32_t& num_rounds, const block& diff, block c0[], block c1[], const bool& Y, const bool& whether_calc_back) {
    word mk[M];
    word rk[MAX_NR];
    block p0, p1;
    for (uint32_t i = 0; i < n; i++) {
        p0 = RAND_BLOCK;
        if (Y) p1.first = p0.first ^ diff.first, p1.second = p0.second ^ diff.second;
        else p1 = RAND_BLOCK;
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, rk, num_rounds);
        encrypt(p0, rk, num_rounds, c0[i]);
        encrypt(p1, rk, num_rounds, c1[i]);
    }
    if (whether_calc_back) {
        for (uint32_t i = 0; i < n; i++) {
            c0[i] = calc_back(c0[i]);
            c1[i] = calc_back(c1[i]);
        }
    }
}

// input diff = (0, 0x40)
// 25 selected bits
// dx[15] || dx[13~12] || dx[10~9] || dx[6~3] || dx[0] || dy[15~1]
uint64_t extract_bits_from_block_simon32_9r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0x8000u) << 9) | ((dx & 0x3000u) << 10) | ((dx & 0x600u) << 11) | ((dx & 0x78u) << 13) | ((dx & 0x1u) << 15);
    res |= (dy & 0xfffeu) >> 1;
    return res;
}

// input diff = (0, 0x40)
// 23 selected bits
// dx[14] || dx[12~10] || dx[8~7] || dx[5] || dx[2~1] || dy[15] || dy[13~8] || dy[6~0]
uint64_t extract_bits_from_block_simon32_10r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0x4000u) << 8) | ((dx & 0x1c00u) << 9) | ((dx & 0x180u) << 10) | ((dx & 0x20u) << 11) | ((dx & 0x6u) << 13);
    res |= ((dy & 0x8000u) >> 2) | ((dy & 0x3f00u) >> 1) | ((dy & 0x7fu));
    return res;
}

// input diff = (0, 0x40)
// 19 selected bits
// dx[15] || dx[13] || dx[10~9] || dx[4~3] || dx[0] || dy[14~10] || dy[8~5] || dy[3~1]
uint64_t extract_bits_from_block_simon32_11r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0x8000u) << 3) | ((dx & 0x2000u) << 4) | ((dx & 0x600u) << 6) | ((dx & 0x18u) << 10) | ((dx & 0x1u) << 12);
    res |= ((dy & 0x7c00u) >> 3) | ((dy & 0x1e0u) >> 2) | ((dy & 0xeu) >> 1);
    return res;
}

// input diff = (0, 0x1)
// 25 selecetd bits
// dx[31~30] || dx[28~27] || dx[8] || dx[5~4] || dx[1~0] || dy[31~22] || dy[19] || dy[7~6] || dy[3~2] || dy[0]
uint64_t extract_bits_from_block_simon64_12r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0xc0000000u) >> 7) | ((dx & 0x18000000u) >> 6) | ((dx & 0x100u) << 12) | ((dx & 0x30u) << 14) | ((dx & 0x3u) << 16);
    res |= ((dy & 0xffc00000u) >> 16) | ((dy & 0x80000u) >> 14) | ((dy & 0xc0u) >> 3) | ((dy & 0xcu) >> 1) | ((dy & 0x1u));
    return res;
}

// input diff = (0, 0x1)
// 18 selected bits
// dx[30~29] || dx[7~6] || dx[3~2] || dy[31~26] || dy[21] || dy[8] || dy[5~4] || dy[1~0]
uint64_t extract_bits_from_block_simon64_13r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0x60000000u) >> 13) | ((dx & 0xc0u) << 8) | ((dx & 0xcu) << 10);
    res |= ((dy & 0xfc000000u) >> 20) | ((dy & 0x200000u) >> 16) | ((dy & 0x100u) >> 4) | ((dy & 0x30u) >> 2) | ((dy & 0x3u));
    return res;
}

// input diff = (0, 0x1)
// 19 selected bits
// dx[63~60] || dx[2] || dx[0] || dy[63~54] || dy[4] || dy[1~0]
uint64_t extract_bits_from_block_simon128_19r(const block& t0, const block& t1) {
    uint64_t dx = t0.first ^ t1.first, dy = t0.second ^ t1.second;
    uint64_t res = ((dx & 0xf000000000000000ull) >> 45) | ((dx & 0x4ull) << 12) | ((dx & 0x1ull) << 13);
    res |= ((dy & 0xffc0000000000000ull) >> 51) | ((dy & 0x10ull) >> 2) | ((dy & 0x3ull));
    return res;
}