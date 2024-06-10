#include "lea.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

bool block::operator==(const block& other) const{
    return (x0 == other.x0) && (x1 == other.x1) && (x2 == other.x2) && (x3 == other.x3);
}
block block::operator^(const block& other) const{
    block res;
    res.x0 = x0 ^ other.x0;
    res.x1 = x1 ^ other.x1;
    res.x2 = x2 ^ other.x2;
    res.x3 = x3 ^ other.x3;
    return res;
}

static word delta[8] = {0xc3efe9dbu, 0x44626b02u, 0x79e27c8aul, 0x78df30ecu, 0x715ea49eu, 0xc785da0au, 0xe04ef22au, 0xe5c40957u};

static word rol(const word& x, int k) {
    k = k % WORD_SIZE;
    return (x << k) | (x >> (WORD_SIZE - k));
}

static word ror(const word& x, int k) {
    k = k % WORD_SIZE;
    return (x >> k) | (x << (WORD_SIZE - k));
}

static block enc_one_round(const block& p, const word rk[6]) {
    block c;
    c.x0 = rol((p.x0 ^ rk[0]) + (p.x1 ^ rk[1]), 9);
    c.x1 = ror((p.x1 ^ rk[2]) + (p.x2 ^ rk[3]), 5);
    c.x2 = ror((p.x2 ^ rk[4]) + (p.x3 ^ rk[5]), 3);
    c.x3 = p.x0;
    return c;
}

static block dec_one_round(const block& c, const word rk[6]) {
    block p;
    p.x0 = c.x3;
    p.x1 = rk[1] ^ (ror(c.x0, 9) - (p.x0 ^ rk[0]));
    p.x2 = rk[3] ^ (rol(c.x1, 5) - (p.x1 ^ rk[2]));
    p.x3 = rk[5] ^ (rol(c.x2, 3) - (p.x2 ^ rk[4]));
    return p;
}

void expand_key(const word mk[], const uint32_t& nr, word rk[][6], const uint32_t& key_bit_length) {
    assert((key_bit_length == 128) || (key_bit_length == 192) || (key_bit_length == 256));
    word T[8];
    word cons;
    if (key_bit_length == 128) {
        memcpy(T, mk, 4 * sizeof(word));
        for (uint32_t i = 0; i < nr; i++) {
            cons = delta[i % 4];
            T[0] = rol(T[0] + rol(cons, i), 1);
            T[1] = rol(T[1] + rol(cons, i + 1), 3);
            T[2] = rol(T[2] + rol(cons, i + 2), 6);
            T[3] = rol(T[3] + rol(cons, i + 3), 11);
            rk[i][0] = T[0];
            rk[i][1] = T[1];
            rk[i][2] = T[2];
            rk[i][3] = T[1];
            rk[i][4] = T[3];
            rk[i][5] = T[1];
        }
    } else if (key_bit_length == 192) {
        memcpy(T, mk, 6 * sizeof(word));
        for (uint32_t i = 0; i < nr; i++) {
            cons = delta[i % 6];
            T[0] = rol(T[0] + rol(cons, i), 1);
            T[1] = rol(T[1] + rol(cons, i + 1), 3);
            T[2] = rol(T[2] + rol(cons, i + 2), 6);
            T[3] = rol(T[3] + rol(cons, i + 3), 11);
            T[4] = rol(T[4] + rol(cons, i + 4), 13);
            T[5] = rol(T[5] + rol(cons, i + 5), 17);
            for (uint32_t j = 0; j < 6; j++) rk[i][j] = T[j];
        }
    } else {
        memcpy(T, mk, 8 * sizeof(word));
        for (uint32_t i = 0; i < nr; i++) {
            cons = delta[i % 8];
            T[(6 * i) % 8] = rol(T[(6 * i) % 8] + rol(cons, i), 1);
            T[(6 * i + 1) % 8] = rol(T[(6 * i + 1) % 8] + rol(cons, i + 1), 3);
            T[(6 * i + 2) % 8] = rol(T[(6 * i + 2) % 8] + rol(cons, i + 2), 6);
            T[(6 * i + 3) % 8] = rol(T[(6 * i + 3) % 8] + rol(cons, i + 3), 11);
            T[(6 * i + 4) % 8] = rol(T[(6 * i + 4) % 8] + rol(cons, i + 4), 13);
            T[(6 * i + 5) % 8] = rol(T[(6 * i + 5) % 8] + rol(cons, i + 5), 17);
            for (uint32_t j = 0; j < 6; j++) rk[i][j] = T[(6 * i + j) % 8];
        }
    }
}

block encrypt(const block& p, const word rk[][6], const uint32_t& nr) {
    block c = p;
    for (uint32_t i = 0; i < nr; i++) c = enc_one_round(c, rk[i]);
    return c;
}

block decrypt(const block& c, const word rk[][6], const uint32_t& nr) {
    block p = c;
    for (int i = nr - 1; i >= 0; i--) p = dec_one_round(p, rk[i]);
    return p;
}

block calculate_back(const block& x) {
    return {x.x3, ror(x.x0, 9), rol(x.x1, 5), rol(x.x2, 3)};
}

bool check_test_vectors(const uint32_t& version) {
    uint32_t key_bit_length;
    uint32_t nr;
    word key[8];
    block p, c, tmp_c, tmp_p;
    word rk[MAX_NR][6];
    if (version == 128) {
        key_bit_length = 128;
        nr = 24;
        key[0] = 0x3c2d1e0fu; key[1] = 0x78695a4b; key[2] = 0xb4a59687u; key[3] = 0xf0e1d2c3u;
        p = {0x13121110u, 0x17161514u, 0x1b1a1918u, 0x1f1e1d1cu};
        c = {0x354ec89fu, 0x18c6c628u, 0xa7c73255u, 0xfd8b6404u};
    } else if (version == 192) {
        key_bit_length = 192;
        nr = 28;
        key[0] = 0x53af3714u; key[1] = 0x75bd6930u; key[2] = 0x0c56c125u; key[3] = 0xa1d2ba78u; key[4] = 0x1c6734e5u; key[5] = 0x7cf27e00u;
        p = {0xcbf4b41cu, 0x51db4b6cu, 0x0984ea68u, 0x51fd7b72u};
        c = {0x6d5c7269u, 0xb7f812f9u, 0xe611b50eu, 0x70583c66u};
    } else {
        key_bit_length = 256;
        nr = 32;
        key[0] = 0xe279674fu; key[1] = 0x19931ebdu; key[2] = 0xac1530c6u; key[3] = 0xa7d7efffu; key[4] = 0x59edf091u; key[5] = 0x07701bdfu; key[6] = 0xe282fe69u; key[7] = 0x358c66f0u;
        p = {0xe3ca31dcu, 0x110a5edau, 0x20b066c9u, 0xdefecfd7u};
        c = {0x2004a2edu, 0xe867f698u, 0xb82da057u, 0xf2dfa7cau};
    }
    expand_key(key, nr, rk, key_bit_length);
    tmp_c = encrypt(p, rk, nr);
    if (c == tmp_c) {
        if (key_bit_length == 128)
            printf("Test vector of LEA-128 is verified.\n");
        else if (key_bit_length == 192)
            printf("Test vector of LEA-192 is verified.\n");
        else
            printf("Test vector of LEA-256 is verified.\n");
    } else {
        printf("Test vector not verified.\n");
        return false;
    }
    tmp_p = decrypt(c, rk, nr);
    if (p == tmp_p) {
        printf("Test decryption function verified.\n");
    } else {
        printf("Test decryption function not verified.\n");
        return false;
    }
    return true;
}