#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "operations.h"

/**
 * Contains a collection of utility and initialization functions for the operations
 * container data structure.
 */

/**
 * An array of operations container data structures, AKA the hash table.
 * Better than using a regular array to store all the assembly code-words
 * because it can be accessed more quickly.
 */
static Operation *opTable[SIZE];

/**
 * Creates and return an hash value based on the given string.
 */
unsigned hash(const char *str) {
	unsigned hash = 0; /* Variable to store the hash value. */
	int length = strlen(str); /* Length of the given string. */
	int i;

	/*
	 * The following hash algorithm is relatively simple and will not necessarily
	 * generate a unique value for every string in existence, but it is enough for
	 * this assembler.
	 * 
	 * This hash was specifically selected for the reason that the hash table
	 * created by it (for this assembler) will not have a row that is longer than
	 * two nodes.
	*/
	for (i = 0; i < length; ++i) {
		hash += str[i] * (length - i) + 2;
	}

	return hash;
}

/**
 * Uses the hash function to relate a string value to an index in the hash table.
 * Used by this assembler to relate an assembly code-word to its relevant data via
 * the hash table.
 */
unsigned getIndex(const char *keyword) {
	return (hash(keyword) % SIZE); /* The index is then returned. */
}

/**
 * Uses the hash function to get the index in which the given keyword is stored.
 * If there was a collision at a certain index this function would search trough
 * the operations container list.
 * The given string is always compared to the strings in the hash table so there
 * will not be a case in which a random string with the same hash value would be
 * recognized as a code-word.
 * Returns a pointer to an operation container node, if the given parameter is a
 * code-word, otherwise a null pointer.
 */
Operation *searchKeyword(const char *keyword) {
	/* Getting the hashed index of the given parameter. */
	Operation *operation = opTable[getIndex(keyword)];

	/* Validating the given parameter. */
	while (operation != NULL) {
		if (strcmp(keyword, operation->keyword) == 0)
			return operation;
		/* The given parameter could be the next code-word in the hash table. */
		operation = operation->next;
	}

	/* The given parameter is not a valid assembly code-word. */
	return NULL;
}

/**
 * Used by the initialization function to insert a code-word into the hash table.
 * Using the given parameters, this function will create a new operations container
 * node and then insert it into the hash table while handling collisions (if there
 * are any).
 * At the end of the process, this function will return a code that tells if the
 * insertion was successful or not.
 */
Code insert(char *keyword, unsigned char type, unsigned char funct, unsigned char opcode) {
	unsigned index;
	/* Initializing the operations container. */
	Operation *operation = malloc(sizeof(Operation));
	Operation *temp; /* Used for handling collisions. */
	/* In case no memory could be allocated an error code will be returned. */
	if (operation == NULL) {
		return ERROR; /* The insertion was not successful. */
	}

	/* Initializing the operations container with the given parameters. */
	operation->keyword = keyword;
	operation->type = type;
	operation->funct = funct;
	operation->opcode = opcode;
	operation->next = NULL; /* For collision handling. */

	/* Relating an index for that code-word. */
	index = getIndex(keyword);
	/* Inserting the operations container into the hash table. */
	if (opTable[index] != NULL) {
		/* Handling a collision. */
		temp = opTable[index];
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = operation;
	} else {
		/* No collision. */
		opTable[index] = operation;
	}

	return SUCCESS; /* The insertion was successful. */
}

/**
 * Inserts all the assembly code-words into the hash table together with every
 * type code, funct code, and opcode.
 * At the end of the process, this function will return a code that tells if the
 * initialization was successful or not.
 */
Code initasmOperations() {
	const unsigned char breakR = 5; /* funct code pattern break for R type code-words. */
	const unsigned char rCount = 8; /* Number of R type code-words. */
	const unsigned char iopcodeStart = 10; /* I type code-words opcode beginning of pattern. */
	const unsigned char iCount = 15; /* Number of I type code-words. */
	const unsigned char jopcodeStart = 30; /* J type code-words opcode beginning of pattern. */
	const unsigned char JCount = 4; /* Number of J type code-words. */
	const unsigned char stopopcode = 63; /* The opcode for the "stop" code-word. */
	int index; /* Used by the loops. */
	Code code = SUCCESS; /* initializing the return code. */
	/*
	 * The following three string arrays contain all the assembly code-words
	 * sorted by their type.
	 */
	char *r[] = { /* R */
		"add", "sub", "and", "or", "nor", "move", "mvhi", "mvlo"
	}, *i[] = { /* I */
		"addi", "subi", "andi", "ori", "nori", "bne", "beq", "blt", "bgt", "lb", "sb", "lw", "sw", "lh", "sh"
	}, *j[] = { /* J */
		"jmp", "la", "call", "stop"
	};

	/*
	 * For all following insertions if one fails all no additional insertions
	 * will be made.
	 */

	/* Sets all indexes in the array of the hash table to a null pointer. */
	for (index = 0; index < SIZE; opTable[index++] = NULL);

	/* Inserts all R type code-words using a funct code pattern. */
	for (index = 0; index < breakR && code == SUCCESS; index++)
		code = insert(r[index], R, (index % FUNCT_COUNT) + 1, 0);
	for (index = breakR; index < rCount && code == SUCCESS; index++)
		code = insert(r[index], R, (index % FUNCT_COUNT) + 1, 1);

	/* Inserts all I type code-words using an opcode pattern. */
	for (index = 0; index < iCount && code == SUCCESS; index++)
		code = insert(i[index], I, NO_FUNCT, index + iopcodeStart);

	/* Inserts all J type code-words using an opcode pattern. */
	for (index = 0; index < (JCount - 1) && code == SUCCESS; index++)
		code = insert(j[index], J, NO_FUNCT, index + jopcodeStart);
	/* Inserts the "stop" code-word. */
	if (code == SUCCESS)
		code = insert(j[JCount - 1], J, NO_FUNCT, stopopcode);

	return code; /* Returning the code after all the insertions. */
}

/**
 * Frees all the the memory used by the hash table and clears it.
 */
void clearasmOperations() {
	int i;
	Operation *temp; /* Temporary variable for collided operations containers. */

	for (i = 0; i < SIZE; i++)
		/* Also clears all nodes formed by collisions. */
		while (opTable[i] != NULL) {
			temp = opTable[i];
			opTable[i] = opTable[i]->next;
			free(temp);
		}
}

void printasm() {
	int i;
	Operation *temp;
	for (i = 0; i < SIZE; i++) {
		printf("[%d]:\n", i);
		temp = opTable[i];
		while (temp != NULL) {
			printf("\t%s: type = %d, funct = %d, opcode = %d\n", temp->keyword, temp->type, temp->funct, temp->opcode);
			temp = temp->next;
		}
	}
}
