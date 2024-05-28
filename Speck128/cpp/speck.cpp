#include <stdio.h>
#include <time.h>
#include "speck.h"

word rol(const word& a, const uint32_t& b) {
    return (a << b) | (a >> (WORD_SIZE - b));
}

word ror(const word& a, const uint32_t& b) {
    return (a >> b) | (a << (WORD_SIZE - b));
}

void enc_one_round(const block& p, const word& k, block& c) {
    word& cl = c.first = p.first;
    word& cr = c.second = p.second;
    cl = ror(cl, ALPHA);
    cl += cr;
    cl ^= k;
    cr = rol(cr, BETA);
    cr ^= cl;
}

void dec_one_round(const block& c, const word& k, block& p) {
    word& pl = p.first = c.first;
    word& pr = p.second = c.second;
    pr ^= pl;
    pr = ror(pr, BETA);
    pl ^= k;
    pl -= pr;
    pl = rol(pl, ALPHA);
}

void expand_key(const word mk[], word keys[], const uint32_t& nr) {
    keys[0] = mk[M - 1];
    word l[MAX_NR + M];
    block inner_state;
    for (int i = 0, j = M - 2; i < M - 1; i++, j--) {
        l[i] = mk[j];
    }
    for (int i = 0, j = M - 1, k = 1; i < nr - 1; i++, j++, k++) {
        enc_one_round(block(l[i], keys[i]), i, inner_state);
        l[j] = inner_state.first;
        keys[k] = inner_state.second;
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

bool check_testvector() {
    // Verify Speck128_128
    #ifdef SPECK128_128
    word mk[M] = {0x0f0e0d0c0b0a0908ull, 0x0706050403020100ull};
    word keys[MAX_NR];
    block p = {0x6c61766975716520ull, 0x7469206564616d20ull};
    block c;
    expand_key(mk, keys, MAX_NR);
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0xa65d985179783265ull, 0x7860fedf5c570d18ull)) {
        printf("SPECK128/128 testvector verified.\n");
    } else {
        printf("SPECK128/128 testvector not verified.\n");
        return false;
    }
    #endif

    // Verify Speck128_192
    #ifdef SPECK128_192
    word mk[M] = {0x1716151413121110ull, 0x0f0e0d0c0b0a0908ull, 0x0706050403020100ull};
    word keys[MAX_NR];
    block p = {0x7261482066656968ull, 0x43206f7420746e65ull};
    block c;
    expand_key(mk, keys, MAX_NR);
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0x1be4cf3a13135566ull, 0xf9bc185de03c1886ull)) {
        printf("SPECK128/192 testvector verified.\n");
    } else {
        printf("SPECK128/192 testvector not verified.\n");
        return false;
    }
    #endif

    // Verify Speck128_256
    #ifdef SPECK128_256
    word mk[M] = {0x1f1e1d1c1b1a1918ull, 0x1716151413121110ull, 0x0f0e0d0c0b0a0908ull, 0x0706050403020100ull};
    word keys[MAX_NR];
    block p = {0x65736f6874206e49ull, 0x202e72656e6f6f70ull};
    block c;
    expand_key(mk, keys, MAX_NR);
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0x4109010405c0f53eull, 0x4eeeb48d9c188f43ull)) {
        printf("SPECK128/256 testvector verified.\n");
    } else {
        printf("SPECK128/256 testvector not verified.\n");
        return false;
    }
    #endif
    
    // Verify decryption
    random_generator.set_rand_seed(time(0));
    uint32_t n = 1 << 20;
    block tmp_p;
    for (uint32_t i = 0; i < n; i++) {
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