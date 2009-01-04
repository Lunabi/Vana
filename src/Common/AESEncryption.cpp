/*
Copyright (C) 2008-2009 Vana Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "AESEncryption.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

AESEncryption *AESEncryption::singleton = 0;

const uint8_t AESEncryption::sbox[256] = {
	//0	 1	2	  3	 4	5	 6	 7	  8	9	 A	  B	C	 D	 E	 F
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, // 0
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, // 1
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, // 2
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, // 3
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, // 4
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, // 5
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, // 6
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, // 7
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, // 8
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, // 9
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, // A
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, // B
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, // C
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, // D
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, // E
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 // F
};

const uint8_t AESEncryption::Rcon[255] = {
	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
	0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
	0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
	0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab,
	0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d,
	0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25,
	0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01,
	0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d,
	0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa,
	0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a,
	0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02,
	0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
	0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
	0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
	0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
	0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f,
	0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5,
	0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33,
	0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb
};

const uint8_t AESEncryption::aes_key[32] = {
	0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xB4,
	0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x33, 0x00,
	0x00, 0x00, 0x52, 0x00, 0x00, 0x00
};

/* Rijndael's key schedule rotate operation
 * rotate the word eight bits to the left
 *
 * rotate(1d2c3a4f) = 2c3a4f1d
 *
 * word is an char array of size 4 (32 bit)
 */
void AESEncryption::rotate(uint8_t *word) {
	uint8_t c;

	c = word[0];
	for (uint8_t i = 0; i < 3; i++) {
		word[i] = word[i+1];
	}
	word[3] = c;
}

void AESEncryption::core(uint8_t *word, int32_t iteration) {
	rotate(word); // Rotate the 32-bit word 8 bits to the left

	for (uint8_t i = 0; i < 4; ++i) { // Apply S-Box substitution on all 4 parts of the 32-bit word
		word[i] = sbox[word[i]];
	}

	// XOR the output of the rcon operation with i to the first part (leftmost) only
	word[0] = word[0]^Rcon[iteration];
}

void AESEncryption::expandKey(uint8_t *expandedKey, enum keySize size, size_t expandedKeySize) {
	/* Rijndael's key expansion
	* expands an 128,192,256 key into an 176,208,240 bytes key
	*
	* expandedKey is a pointer to an char array of large enough size
	* key is a pointer to a non-expanded key
	*/

	int32_t currentSize = 0; // Current expanded keySize, in bytes
	int32_t rconIteration = 1;
	uint8_t t[4] = {0}; // Temporary 4-byte variable

	for (uint8_t i = 0; i < size; i++) { // Set the 16,24,32 bytes of the expanded key to the input key
		expandedKey[i] = aes_key[i];
	}
	currentSize += size;

	while ((unsigned) currentSize < (unsigned) expandedKeySize) 	{
		for (uint8_t i = 0; i < 4; i++) { // Assign the previous 4 bytes to the temporary value t
			t[i] = expandedKey[(currentSize - 4) + i];
		}

		if (currentSize % size == 0) { // Every 16,24,32 bytes apply the core schedule to t and increment rconIteration afterwards
			core(t, rconIteration++);
		}

		if (size == SIZE_32 && ((currentSize % size) == 16)) { // For 256-bit keys, add an extra sbox to the calculation
			for (uint8_t i = 0; i < 4; i++) {
				t[i] = sbox[t[i]];
			}
		}

		for (uint8_t i = 0; i < 4; i++) { // XOR t with the four-byte block 16,24,32 bytes before the new expanded key. This becomes the next four bytes in the expanded key.
			expandedKey[currentSize] = expandedKey[currentSize - size] ^ t[i];
			currentSize++;
		}
	}
}

void AESEncryption::subBytes(uint8_t *state) {
	for (uint8_t i = 0; i < 16; i++) { // substitute all the values from the state with the value in the SBox using the state value as index for the SBox
		state[i] = sbox[state[i]];
	}
}

void AESEncryption::shiftRow(uint8_t *state, uint8_t nbr) {
	uint8_t tmp;
	for (uint8_t i = 0; i < nbr; i++) { // Each iteration shifts the row to the left by 1
		tmp = state[0];
		for (uint8_t j = 0; j < 3; j++) {
			state[j] = state[j+1];
		}
		state[3] = tmp;
	}
}

void AESEncryption::shiftRows(uint8_t *state) {
	for (uint8_t i = 0; i < 4; i++) { // Iterate over the 4 rows and call shiftRow() with that row
		shiftRow(state+i*4, i);
	}
}

void AESEncryption::addRoundKey(uint8_t *state, uint8_t *roundKey) {
	for (uint8_t i = 0; i < 16; i++) {
		state[i] = state[i] ^ roundKey[i];
	}
}

uint8_t AESEncryption::galois_multiplication(uint8_t a, uint8_t b) {
	uint8_t p = 0;
	uint8_t hi_bit_set;
	for (uint8_t counter = 0; counter < 8; counter++) {
		if ((b & 1) == 1)
			p ^= a;
		hi_bit_set = (a & 0x80);
		a <<= 1;
		if (hi_bit_set == 0x80)
			a ^= 0x1b;
		b >>= 1;
	}
	return p;
}

void AESEncryption::mixColumn(uint8_t *column) {
	uint8_t cpy[4];
	for (uint8_t i = 0; i < 4; i++) {
		cpy[i] = column[i];
	}
	column[0] = galois_multiplication(cpy[0], 2) ^
				galois_multiplication(cpy[3], 1) ^
				galois_multiplication(cpy[2], 1) ^
				galois_multiplication(cpy[1], 3);

	column[1] = galois_multiplication(cpy[1], 2) ^
				galois_multiplication(cpy[0], 1) ^
				galois_multiplication(cpy[3], 1) ^
				galois_multiplication(cpy[2], 3);

	column[2] = galois_multiplication(cpy[2], 2) ^
				galois_multiplication(cpy[1], 1) ^
				galois_multiplication(cpy[0], 1) ^
				galois_multiplication(cpy[3], 3);

	column[3] = galois_multiplication(cpy[3], 2) ^
				galois_multiplication(cpy[2], 1) ^
				galois_multiplication(cpy[1], 1) ^
				galois_multiplication(cpy[0], 3);
}

void AESEncryption::mixColumns(uint8_t *state) {
	uint8_t column[4];

	for (uint8_t i = 0; i < 4; i++) { // Iterate over the 4 columns
		for (uint8_t j = 0; j < 4; j++) { // Construct one column by iterating over the 4 rows
			column[j] = state[(j*4)+i];
		}

		mixColumn(column); // Apply the mixColumn on one column

		for (uint8_t j = 0; j < 4; j++) { // Put the values back into the state
			state[(j*4)+i] = column[j];
		}
	}
}

void AESEncryption::aes_round(uint8_t *state, uint8_t *roundKey) {
	subBytes(state);
	shiftRows(state);
	mixColumns(state);
	addRoundKey(state, roundKey);
}

void AESEncryption::createRoundKey(uint8_t *expandedKey, uint8_t *roundKey) {
	for (uint8_t i = 0; i < 4; i++) { // Iterate over the columns
		for (uint8_t j = 0; j < 4; j++) { // Iterate over the rows
			roundKey[(i+(j*4))] = expandedKey[(i*4)+j];
		}
	}
}

void AESEncryption::aes_main(uint8_t *state, uint8_t *expandedKey, int32_t nbrRounds) {
	uint8_t roundKey[16];

	createRoundKey(expandedKey, roundKey);
	addRoundKey(state, roundKey);

	for (uint8_t i = 1; i < nbrRounds; i++) {
		createRoundKey(expandedKey + 16*i, roundKey);
		aes_round(state, roundKey);
	}

	createRoundKey(expandedKey + 16*nbrRounds, roundKey);
	subBytes(state);
	shiftRows(state);
	addRoundKey(state, roundKey);
}

int8_t AESEncryption::aes_encrypt(uint8_t *input, uint8_t *output, enum keySize size) {
	int32_t expandedKeySize; // The expanded keySize
	int32_t nbrRounds; // The number of rounds
	uint8_t *expandedKey; // The expanded key
	uint8_t block[16]; // The 128 bit block to encode

	switch (size) { // Set the number of rounds
		case SIZE_16:
			nbrRounds = 10;
			break;
		case SIZE_24:
			nbrRounds = 12;
			break;
		case SIZE_32:
			nbrRounds = 14;
			break;
		default:
			return -2;
			break;
	}

	expandedKeySize = (16*(nbrRounds+1));

	expandedKey = new uint8_t[expandedKeySize * sizeof(uint8_t)];

	/* Set the block values, for the block:
	 * a0,0 a0,1 a0,2 a0,3
	 * a1,0 a1,1 a1,2 a1,3
	 * a2,0 a2,1 a2,2 a2,3
	 * a3,0 a3,1 a3,2 a3,3
	 * the mapping order is a0,0 a1,0 a2,0 a3,0 a0,1 a1,1 ... a2,3 a3,3
	 */

	for (uint8_t i = 0; i < 4; i++)	{ // Iterate over the columns
		for (uint8_t j = 0; j < 4; j++) { // Iterate over the rows
			block[(i+(j*4))] = input[(i*4)+j];
		}
	}

	expandKey(expandedKey, size, expandedKeySize); // Expand the key into an 176, 208, 240 bytes key
	aes_main(block, expandedKey, nbrRounds); // Encrypt the block using the expandedKey

	for (uint8_t i = 0; i < 4; i++) { // Unmap the block again into the output
		for (uint8_t j = 0; j < 4; j++) { // Iterate over the rows
			output[(i*4)+j] = block[(i+(j*4))];
		}
	}
	delete[] expandedKey;
	return 0;
}

void AESEncryption::decryptofb(uint8_t *buffer, uint8_t *vec, int32_t bsize) {
	// The AES input/output
	uint8_t ciphertext[16] = {0};
	uint8_t input[16] = {0};
	uint8_t output[16] = {0};
	uint8_t plaintext[16] = {0};
	uint8_t IV[16];

	for (uint8_t i = 0; i < 16; i++) {
		IV[i] = vec[i % 4];
	}

 	for (uint8_t j = 0; j < (bsize / 16 + 1); j++) {
		if (j == 0) {
			aes_encrypt(IV, output, SIZE_32);
		}
		else {
			aes_encrypt(input, output, SIZE_32);
		}
		for (uint8_t i = 0; i < 16; i++) {
			plaintext[i] = output[i] ^ buffer[j*16+i];
		}
		if (j == bsize / 16)
			memcpy(buffer+(j * 16), plaintext, bsize % 16);
		else
			memcpy(buffer+(j * 16), plaintext, 16);

		memcpy(input, output, 16 * sizeof(uint8_t));
	}
}
