#include "simon.h"
#include <stdio.h>
#include <time.h>

static word z[5][62] = {
    {1,1,1,1,1,0,1,0,0,0,1,0,0,1,0,1,0,1,1,0,0,0,0,1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,0,0,1,0,0,1,0,1,0,1,1,0,0,0,0,1,1,1,0,0,1,1,0},
    {1,0,0,0,1,1,1,0,1,1,1,1,1,0,0,1,0,0,1,1,0,0,0,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1,0,1,1,1,1,1,0,0,1,0,0,1,1,0,0,0,0,1,0,1,1,0,1,0},
    {1,0,1,0,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,1,0,1,1,0,1,1,0,0,1,1},
    {1,1,0,1,1,0,1,1,1,0,1,0,1,1,0,0,0,1,1,0,0,1,0,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,1,0,0,1,1,0,1,0,0,0,0,1,1,1,1},
    {1,1,0,1,0,0,0,1,1,1,1,0,0,1,1,0,1,0,1,1,0,1,1,0,0,0,1,0,0,0,0,0,0,1,0,1,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,1,1,1,0,1,1,1,1}
};

word rol(const word& a, const uint32_t& b) {
    return ((a << b) & MASK_VAL) | (a >> (WORD_SIZE - b));
}

word ror(const word& a, const uint32_t& b) {
    return (a >> b) | ((a << (WORD_SIZE - b)) & MASK_VAL);
}

void enc_one_round(const block& p, const word& k, block& c) {
    word tmp = p.first;
    c.first = (rol(tmp, ALPHA) & rol(tmp, BETA)) ^ rol(tmp, GAMMA) ^ k ^ p.second;
    c.second = tmp;
}

void dec_one_round(const block& c, const word& k, block& p) {
    word tmp = c.second;
    p.second = (rol(tmp, ALPHA) & rol(tmp, BETA)) ^ rol(tmp, GAMMA) ^ k ^ c.first;
    p.first = tmp;
}

void expand_key(const word mk[], word keys[], const uint32_t& nr) {
    word tmp;
    for (int i = 0; i < M; i++) {
        keys[i] = mk[M - 1 - i];
    }
    for (int i = M; i < MAX_NR; i++) {
        tmp = ror(keys[i-1], 3);
        if (M == 4) tmp ^= keys[i-3];
        tmp = tmp ^ ror(tmp, 1);
        keys[i] = (keys[i-M] ^ MASK_VAL) ^ tmp ^ z[CONST_INDEX][(i - M) % 62] ^ 3;
    }
}

void encrypt(const block& p, const word keys[], const uint32_t& nr, block& c) {
    c = p;
    for (int i = 0; i < nr; i++) {
        enc_one_round(c, keys[i], c);
    }
}

void decrypt(const block& c, const word keys[], const uint32_t& nr, block& p) {
    p = c;
    for (int i = nr - 1; i >= 0; i--) {
        dec_one_round(p, keys[i], p);
    }
}

block calc_back(const block& c) {
    return block(c.second, (rol(c.second, ALPHA) & rol(c.second, BETA)) ^ rol(c.second, GAMMA) ^ c.first);
}

bool check_testvector() {
    // Verify Simon32/64
    #ifdef SIMON32_64
    word mk[M] = {0x1918u, 0x1110u, 0x0908u, 0x0100u};
    word keys[MAX_NR];
    expand_key(mk, keys, MAX_NR);
    block p = {0x6565u, 0x6877u};
    block c;
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0xc69bu, 0xe9bbu)) {
        printf("Simon32/64 test vector verified.\n");
    } else {
        printf("Simon32/64 test vector not verified.\n");
        return false;
    }
    #endif

    // Verify Simon64/128
    #ifdef SIMON64_128
    word mk[M] = {0x1b1a1918u, 0x13121110u, 0x0b0a0908u, 0x03020100u};
    word keys[MAX_NR];
    expand_key(mk, keys, MAX_NR);
    block p = {0x656b696cu, 0x20646e75};
    block c;
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0x44c8fc20u, 0xb9dfa07au)) {
        printf("Simon64/128 test vector verified.\n");
    } else {
        printf("Simon64/128 test vector not verified.\n"); 
        return false;
    }
    #endif

    // Verify Simon128/128
    #ifdef SIMON128_128
    word mk[M] = {0x0f0e0d0c0b0a0908ull, 0x0706050403020100ull};
    word keys[MAX_NR];
    expand_key(mk, keys, MAX_NR);
    block p = {0x6373656420737265ull, 0x6c6c657661727420ull};
    block c;
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0x49681b1e1e54fe3full, 0x65aa832af84e0bbcull)) {
        printf("Simon128/128 test vector verifed.\n");
    } else {
        printf("Simon128/128 test vector not verifed.\n");
        return false;
    }
    #endif

    // Verify the decryption function
    random_generator.set_rand_seed(time(0));
    int n; 
    n = (1 << 20);
    block tmp_p;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) mk[j] = RAND_WORD;
        expand_key(mk, keys, MAX_NR);
        p = RAND_BLOCK;
        encrypt(p, keys, MAX_NR, c);
        decrypt(c, keys, MAX_NR, tmp_p);
        if (p != tmp_p) {
            printf("Test decryption not verified.\n");
            return false;
        }
    }
    printf("Test decryption verified.\n");
    return true;
}