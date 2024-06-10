#include "rand_gen.h"
#include <string.h>
#include <stdio.h>
using namespace std;
inline uint64_t RandomGenerator::rotate_left(uint64_t x, unsigned n)
{
	return (x << n) | (x >> (64 - n));
}
inline void RandomGenerator::transform(uint64_t* A)
{
	for (int round = 0; round < 24; round++)
	{
		uint64_t C[5], D[5];
		C[0] = A[0 * 5 + 0] ^ A[1 * 5 + 0] ^ A[2 * 5 + 0] ^ A[3 * 5 + 0] ^ A[4 * 5 + 0];
		C[1] = A[0 * 5 + 1] ^ A[1 * 5 + 1] ^ A[2 * 5 + 1] ^ A[3 * 5 + 1] ^ A[4 * 5 + 1];
		C[2] = A[0 * 5 + 2] ^ A[1 * 5 + 2] ^ A[2 * 5 + 2] ^ A[3 * 5 + 2] ^ A[4 * 5 + 2];
		C[3] = A[0 * 5 + 3] ^ A[1 * 5 + 3] ^ A[2 * 5 + 3] ^ A[3 * 5 + 3] ^ A[4 * 5 + 3];
		C[4] = A[0 * 5 + 4] ^ A[1 * 5 + 4] ^ A[2 * 5 + 4] ^ A[3 * 5 + 4] ^ A[4 * 5 + 4];

		D[0] = C[4] ^ rotate_left(C[1], 1);
		D[1] = C[0] ^ rotate_left(C[2], 1);
		D[2] = C[1] ^ rotate_left(C[3], 1);
		D[3] = C[2] ^ rotate_left(C[4], 1);
		D[4] = C[3] ^ rotate_left(C[0], 1);

		uint64_t B00 = A[0 * 5 + 0] ^ D[0];
		uint64_t B10 = rotate_left(A[0 * 5 + 1] ^ D[1], 1);
		uint64_t B20 = rotate_left(A[0 * 5 + 2] ^ D[2], 62);
		uint64_t B5 = rotate_left(A[0 * 5 + 3] ^ D[3], 28);
		uint64_t B15 = rotate_left(A[0 * 5 + 4] ^ D[4], 27);

		uint64_t B16 = rotate_left(A[1 * 5 + 0] ^ D[0], 36);
		uint64_t B1 = rotate_left(A[1 * 5 + 1] ^ D[1], 44);
		uint64_t B11 = rotate_left(A[1 * 5 + 2] ^ D[2], 6);
		uint64_t B21 = rotate_left(A[1 * 5 + 3] ^ D[3], 55);
		uint64_t B6 = rotate_left(A[1 * 5 + 4] ^ D[4], 20);

		uint64_t B7 = rotate_left(A[2 * 5 + 0] ^ D[0], 3);
		uint64_t B17 = rotate_left(A[2 * 5 + 1] ^ D[1], 10);
		uint64_t B2 = rotate_left(A[2 * 5 + 2] ^ D[2], 43);
		uint64_t B12 = rotate_left(A[2 * 5 + 3] ^ D[3], 25);
		uint64_t B22 = rotate_left(A[2 * 5 + 4] ^ D[4], 39);

		uint64_t B23 = rotate_left(A[3 * 5 + 0] ^ D[0], 41);
		uint64_t B8 = rotate_left(A[3 * 5 + 1] ^ D[1], 45);
		uint64_t B18 = rotate_left(A[3 * 5 + 2] ^ D[2], 15);
		uint64_t B3 = rotate_left(A[3 * 5 + 3] ^ D[3], 21);
		uint64_t B13 = rotate_left(A[3 * 5 + 4] ^ D[4], 8);

		uint64_t B14 = rotate_left(A[4 * 5 + 0] ^ D[0], 18);
		uint64_t B24 = rotate_left(A[4 * 5 + 1] ^ D[1], 2);
		uint64_t B9 = rotate_left(A[4 * 5 + 2] ^ D[2], 61);
		uint64_t B19 = rotate_left(A[4 * 5 + 3] ^ D[3], 56);
		uint64_t B4 = rotate_left(A[4 * 5 + 4] ^ D[4], 14);

		A[0 * 5 + 0] = B00 ^ ((~B1) & B2);
		A[0 * 5 + 1] = B1 ^ ((~B2) & B3);
		A[0 * 5 + 2] = B2 ^ ((~B3) & B4);
		A[0 * 5 + 3] = B3 ^ ((~B4) & B00);
		A[0 * 5 + 4] = B4 ^ ((~B00) & B1);

		A[1 * 5 + 0] = B5 ^ ((~B6) & B7);
		A[1 * 5 + 1] = B6 ^ ((~B7) & B8);
		A[1 * 5 + 2] = B7 ^ ((~B8) & B9);
		A[1 * 5 + 3] = B8 ^ ((~B9) & B5);
		A[1 * 5 + 4] = B9 ^ ((~B5) & B6);

		A[2 * 5 + 0] = B10 ^ ((~B11) & B12);
		A[2 * 5 + 1] = B11 ^ ((~B12) & B13);
		A[2 * 5 + 2] = B12 ^ ((~B13) & B14);
		A[2 * 5 + 3] = B13 ^ ((~B14) & B10);
		A[2 * 5 + 4] = B14 ^ ((~B10) & B11);

		A[3 * 5 + 0] = B15 ^ ((~B16) & B17);
		A[3 * 5 + 1] = B16 ^ ((~B17) & B18);
		A[3 * 5 + 2] = B17 ^ ((~B18) & B19);
		A[3 * 5 + 3] = B18 ^ ((~B19) & B15);
		A[3 * 5 + 4] = B19 ^ ((~B15) & B16);

		A[4 * 5 + 0] = B20 ^ ((~B21) & B22);
		A[4 * 5 + 1] = B21 ^ ((~B22) & B23);
		A[4 * 5 + 2] = B22 ^ ((~B23) & B24);
		A[4 * 5 + 3] = B23 ^ ((~B24) & B20);
		A[4 * 5 + 4] = B24 ^ ((~B20) & B21);

		A[0] ^= RC[round];
	}
}
RandomGenerator::RandomGenerator(uint64_t seed) {
	set_rand_seed(seed);
}
void RandomGenerator::set_rand_seed(uint64_t seed) {
	pos = -1;
	memset(A, 0, sizeof(A));
	A[0] = seed;
	transform(A);
}
uint8_t RandomGenerator::rand_byte() {
	if (pos == 199) {
		pos = -1;
		transform(A);
	}
	++pos;
	return (A[pos >> 3] >> shift_num[pos & 0x7]) & 0xff;
}

RandomGenerator random_generator(23);