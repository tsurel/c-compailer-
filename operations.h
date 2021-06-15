#pragma once

#define R 1 /* Operation code for type R. */
#define I 2 /* Operation code for type I. */
#define J 3 /* Operation code for type J. */
#define FUNCT_COUNT 5
#define NO_FUNCT 0

#define SIZE 27 /* Number of assembly operations. */

/**
 * Defining the operations container data structure.
 * This structure is used by this assembler for storing, accessing, and
 * differentiating assembly code-words.
 * This structure is used ass a part of an hash table.
 */
typedef struct op {
	char *keyword; /* Operation symbol in assembly code (code-word). */
	unsigned char type; /* R/I/J */
	unsigned char funct; /* Not zero for operations that have the same opcode. */
	unsigned char opcode; /* Operation identification code. */
	struct op *next; /* Points to the next operation with the same hash value. */
} Operation;

unsigned hash(char *str);
unsigned getIndex(char *keyword);
unsigned char insert(char *keyword, unsigned char type, unsigned char funct, unsigned char opcode);
unsigned char initasmOperations();
void clearasmOperations();
void printasm();
