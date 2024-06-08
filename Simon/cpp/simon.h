#ifndef SIMON_H
#define SIMON_H

#include <stdint.h>
#include <utility>
#include <stdlib.h>
#include "rand_gen.h"
using namespace std;

#define ALPHA 1
#define BETA 8
#define GAMMA 2

#define RAND_BYTE (random_generator.rand_byte())

// Choose version of Simon cipher here!
// #define SIMON32_64
// #define SIMON64_128
#define SIMON128_128

#ifdef SIMON32_64
#define WORD_SIZE 16
#define MASK_VAL 0xffffu
#define MAX_NR 32
#define M 4
#define CONST_INDEX 0
#define RAND_WORD (random_generator.rand_16bits())
typedef uint16_t word;
#endif

#ifdef SIMON64_128
#define WORD_SIZE 32
#define MASK_VAL 0xffffffffu
#define MAX_NR 44
#define M 4
#define CONST_INDEX 3
#define RAND_WORD (random_generator.rand_32bits())
typedef uint32_t word;
#endif

#ifdef SIMON128_128
#define WORD_SIZE 64
#define MASK_VAL 0xffffffffffffffffull
#define MAX_NR 68
#define M 2
#define CONST_INDEX 2
#define RAND_WORD (random_generator.rand_64bits())
typedef uint64_t word;
#endif

typedef pair<word, word> block;
#define RAND_BLOCK {RAND_WORD, RAND_WORD}

word rol(const word& a, const uint32_t& b);
word ror(const word& a, const uint32_t& b);
void enc_one_round(const block& p, const word& k, block& c);
void dec_one_round(const block& c, const word& k, block& p);
void expand_key(const word mk[], word keys[], const uint32_t& nr);
void encrypt(const block& p, const word keys[], const uint32_t& nr, block& c);
void decrypt(const block& c, const word keys[], const uint32_t& nr, block& p);
block calc_back(const block& c);
bool check_testvector();

#endif