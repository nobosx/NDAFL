#pragma once
#include <stdint.h>

#define WORD_SIZE 32
#define MASK_VAL 0x80000000u
#define MAX_NR 32

typedef uint32_t word;
struct block {
    word x0;
    word x1;
    word x2;
    word x3;
    bool operator==(const block& other) const;
    block operator^(const block& other) const;
};

bool check_test_vectors(const uint32_t& version=128);
block encrypt(const block& p, const word rk[][6], const uint32_t& nr);
block decrypt(const block& c, const word rk[][6], const uint32_t& nr);
void expand_key(const word mk[], const uint32_t& nr, word rk[][6], const uint32_t& key_bit_length=128);
block calculate_back(const block& x);