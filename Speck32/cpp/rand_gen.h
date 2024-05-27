#pragma once
#include <stdint.h>

class RandomGenerator {
    private:
        inline uint64_t rotate_left(uint64_t x, unsigned n);
        uint64_t A[25];
        int pos;
        const uint64_t RC[24] = {
            0x0000000000000001ull, 0x0000000000008082ull, 0x800000000000808Aull, 0x8000000080008000ull,
            0x000000000000808Bull, 0x0000000080000001ull, 0x8000000080008081ull, 0x8000000000008009ull,
            0x000000000000008Aull, 0x0000000000000088ull, 0x0000000080008009ull, 0x000000008000000Aull,
            0x000000008000808Bull, 0x800000000000008Bull, 0x8000000000008089ull, 0x8000000000008003ull,
            0x8000000000008002ull, 0x8000000000000080ull, 0x000000000000800Aull, 0x800000008000000Aull,
            0x8000000080008081ull, 0x8000000000008080ull, 0x0000000080000001ull, 0x8000000080008008ull
        };
        inline void transform(uint64_t* A);
        const uint32_t shift_num[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    public:
        void set_rand_seed(uint64_t);
        uint8_t rand_byte();
        RandomGenerator(uint64_t);
};

extern RandomGenerator random_generator;