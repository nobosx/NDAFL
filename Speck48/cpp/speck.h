#ifndef SPECK_H
#define SPECK_H

#include <stdint.h>
#include <utility>
#include <stdlib.h>
#include "rand_gen.h"
using namespace std;

typedef uint32_t word;
typedef pair<word, word> block;

#define WORD_SIZE 24
#define ALPHA 8
#define BETA 3
#define MASK_VAL 0xfffffful

// Choose the version of Speck48
#define SPECK48_72
// #define SPECK48_96

#ifdef SPECK48_72
#define M 3
#define MAX_NR 22
#else
#define M 4
#define MAX_NR 23
#endif

#define RAND_SEED 12
#define RAND_BYTE (random_generator.rand_byte())
#define RAND_WORD (random_generator.rand_32bits() & MASK_VAL)
#define RAND_BLOCK {RAND_WORD, RAND_WORD}

word rol(const word& a, const uint32_t& b);
word ror(const word& a, const uint32_t& b);
void enc_one_round(const block& p, const word& k, block& c);
void dec_one_round(const block& c, const word& k, block& p);
void expand_key(const word mk[], word keys[], const uint32_t& nr);
void encrypt(const block& p, const word keys[], const uint32_t& nr, block& c);
void decrypt(const block& c, const word keys[], const uint32_t& nr, block& p);
bool check_testvector();

#endif