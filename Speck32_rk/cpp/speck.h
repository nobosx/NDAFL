#pragma once
#include <stdint.h>
#include <utility>
#include <stdlib.h>
#include "rand_gen.h"
using namespace std;

typedef uint16_t word;
typedef pair<word, word> block;

#define WORD_SIZE 16
#define ALPHA 7
#define BETA 2
#define MASK_VAL 0xffff
#define M 4
#define MAX_NR 50
#define RAND_SEED 12
#define RAND_BYTE (random_generator.rand_byte())
#define RAND_WORD ((word(RAND_BYTE) << 8) | RAND_BYTE)

word rol(const word& a, const uint32_t& b);
word ror(const word& a, const uint32_t& b);
void enc_one_round(const block& p, const word& k, block& c);
void dec_one_round(const block& c, const word& k, block& p);
void expand_key(const word mk[], word keys[], const uint32_t& nr);
void encrypt(const block& p, const word keys[], const uint32_t& nr, block& c);
void decrypt(const block& c, const word keys[], const uint32_t& nr, block& p);
bool check_testvector();