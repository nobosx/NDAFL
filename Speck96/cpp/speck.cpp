#include <stdio.h>
#include <time.h>
#include "speck.h"

word rol(const word& a, const uint32_t& b) {
    return ((a << b) & MASK_VAL) | ((a & MASK_VAL) >> (WORD_SIZE - b));
}

word ror(const word& a, const uint32_t& b) {
    return ((a & MASK_VAL) >> b) | ((a << (WORD_SIZE - b)) & MASK_VAL);
}

void enc_one_round(const block& p, const word& k, block& c) {
    word& cl = c.first = p.first;
    word& cr = c.second = p.second;
    cl = ror(cl, ALPHA);
    cl = (cl + cr) & MASK_VAL;
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
    pl = (pl - pr) & MASK_VAL;
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
    // Verify Speck96_96
    #ifdef SPECK96_96
    printf("Test Speck96-96...\n");
    word mk[M] = {0x0d0c0b0a0908ull, 0x050403020100ull};
    block p = {0x65776f68202cull, 0x656761737520ull};
    word keys[MAX_NR];
    expand_key(mk, keys, MAX_NR);
    block c;
    encrypt(p, keys, MAX_NR, c);
    if (c == block(0x9e4d09ab7178ull, 0x62bdde8f79aaull)) {
        printf("Testvector verified.\n");
    } else {
        printf("Testvector not verified.\n");
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