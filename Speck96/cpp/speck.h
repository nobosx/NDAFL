#ifndef SPECK_H
#define SPECK_H

#include <stdint.h>
#include <utility>
#include <stdlib.h>
#include "rand_gen.h"
using namespace std;

typedef uint64_t word;
typedef pair<word, word> block;

#define WORD_SIZE 48
#define ALPHA 8
#define BETA 3
#define MASK_VAL 0xffffffffffffull

// Choose the version of Speck96
#define SPECK96_96
// #define SPECK96_144

#ifdef SPECK96_96
#define M 2
#define MAX_NR 28
#else
#define M 3
#define MAX_NR 29
#endif

#define RAND_SEED 12
#define RAND_BYTE (random_generator.rand_byte())
#define RAND_WORD (random_generator.rand_64bits() & MASK_VAL)
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