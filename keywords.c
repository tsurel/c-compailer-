#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "keywords.h"
#include "asmutils.h"

/**
 * Contains a collection of utility and initialization functions for the operators
 * and instructors data structures. This translation unit is used by the assembler
 * for relating strings from assembly source files to their codes so they can be
 * distinguished from labels and assembled in a specific way.
 */

#define FUNCT_COUNT 5 /* Number of existing funct codes. */
#define NO_FUNCT 0 /* For operators that do not have funct code. */
#define OP_COUNT 27 /* Number of assembly operators. */
#define INS_COUNT 6 /* Number of assembly instructors. */

/**
 * Defining the operator data structure.
 * This structure is used by this assembler for storing, accessing, and
 * differentiating assembly operator keywords.
 */
struct op {
	char *keyword; /* Operator symbol in assembly code (keyword). */
	unsigned char type; /* R/I/J macros defined in header.*/
	unsigned char funct; /* Not zero for operators that have the same opcode. */
	unsigned char opcode; /* Operator identification code. */
};

/**
 * Defining the instructor data structure.
 * This structure is used by this assembler for storing, accessing, and
 * differentiating assembly Instructor keywords.
 */
struct ins {
	char *keyword; /* Instructor symbol in assembly code (keyword). */
	Expectation expecting; /* To track what is expected after an instruction. */
};

/**
 * The following functions should not be used outside of this translation unit.
 */
Code insertOperator(char *keyword, unsigned char type, unsigned char funct, unsigned char opcode, int index);
Code insertInstructor(char *keyword, Expectation expecting, int index);

/**
 * An array of operators data structures pointers.
 */
static Operator *operators[OP_COUNT];

/**
 * An array of instructors data structures pointers.
 */
static Instructor *instructors[INS_COUNT];

/**
 * Returns the type code of the given operator pointer.
 */
unsigned char getType(Operator *operator) {
	return operator->type;
}

/**
 * Returns the funct code of the given operator pointer.
 */
unsigned char getFunct(Operator *operator) {
	return operator->funct;
}

/**
 * Returns the opcode code of the given operator pointer.
 */
unsigned char getOpcode(Operator *operator) {
	return operator->opcode;
}

/**
 * Returns the expectation of the given instructor pointer.
 */
Expectation getExpectation(Instructor *instructor) {
	return instructor->expecting;
}

/**
 * Searches for the related operator object using its string representation.
 * Returns a pointer to the operator object, if the given parameter is a valid
 * keyword, otherwise a null pointer.
 */
Operator *searchOperatorByString(const char *keyword) {
	int index;

	/* Searching trough the operators array. */
	for (index = 0; index < OP_COUNT; index++)
		/* Works by comparing every string in the array to the given one. */
		if (strcmp(operators[index]->keyword, keyword) == 0)
			/* The given parameter is a valid assembly operator. */
			return operators[index];

	/* The given parameter is not a valid assembly operator. */
	return NULL;

}

/**
 * Searches for the related instructor object using its string representation.
 * Returns a pointer to the instructor object, if the given parameter is a valid
 * keyword, otherwise a null pointer.
 */
Instructor *searchInstructorByString(const char *keyword) {
	int index;

	/* Searching trough the instructors array. */
	for (index = 0; index < INS_COUNT; index++)
		/* Works by comparing every string in the array to the given one. */
		if (strcmp(instructors[index]->keyword, keyword) == 0)
			/* The given parameter is a valid assembly instructor. */
			return instructors[index];
	
	/* The given parameter is not a valid assembly instructor. */
	return NULL;
}

/**
 * Used by the initialization function to insert an operator into the operators array at
 * a specified index.
 * Using the given parameters, this function will create a new operator object in memory
 * and then insert it into the operators array.
 * At the end of the process, this function will return a code that tells if the
 * insertion was successful or not.
 */
Code insertOperator(char *keyword, unsigned char type, unsigned char funct, unsigned char opcode, int index) {
	/* Initializing the operator. */
	Operator *operator = malloc(sizeof(Operator));
	/* In case no memory could be allocated an error code would be returned. */
	if (operator == NULL) {
		return ERROR; /* The insertion was not successful. */
	}

	/* Initializing the operator with the given parameters. */
	operator->keyword = keyword;
	operator->type = type;
	operator->funct = funct;
	operator->opcode = opcode;
	
	/* Inserting the operator into the array at the given index. */
	operators[index] = operator;

	return SUCCESS; /* The insertion was successful. */
}

/**
 * Used by the initialization function to insert an instructor into the instructors array
 * at a specified index.
 * Using the given parameters, this function will create a new instructor object in memory
 * and then insert it into the instructors array.
 * At the end of the process, this function will return a code that tells if the
 * insertion was successful or not.
 */
Code insertInstructor(char *keyword, Expectation expecting, int index) {
	/* Initializing the instructor. */
	Instructor *instructor = malloc(sizeof(Instructor));
	/* In case no memory could be allocated an error code would be returned. */
	if (instructor == NULL) {
		return ERROR; /* The insertion was not successful. */
	}

	/* Initializing the instructor with the given parameters. */
	instructor->keyword = keyword;
	instructor->expecting = expecting;

	/* Inserting the instructor into the array at the given index. */
	instructors[index] = instructor;

	return SUCCESS; /* The insertion was successful. */
}

/**
 * Initializes all the assembly keywords so the assembler could link the
 * string representation of an operator or an instructor to its related
 * codes, that way it can be assembled.
 * At the end of the process, this function will return a code that tells
 * if the initialization was successful or not.
 * This function should be called only once.
 */
Code initasmKeywords() {
	const unsigned char breakR = 5; /* funct code pattern break for R type keywords. */
	const unsigned char rCount = 8; /* Number of R type operators. */
	const unsigned char iopcodeStart = 10; /* I type operators opcode beginning of pattern. */
	const unsigned char iCount = 15; /* Number of I type keywords. */
	const unsigned char jopcodeStart = 30; /* J type operators opcode beginning of pattern. */
	const unsigned char jCount = 4; /* Number of J type operators. */
	const unsigned char stopopcode = 63; /* The opcode for the "stop" keyword. */
	int index; /* Used by the loops. */
	int operatorsIndex = 0; /* Tracks the position for every operator in the operators array. */
	Code code = SUCCESS; /* initializing the return code. */
	/*
	 * The following four string arrays contain all the assembly keywords
	 * sorted by their type.
	 */
	char *r[] = { /* R */
		"add", "sub", "and", "or", "nor", "move", "mvhi", "mvlo"
	}, *i[] = { /* I */
		"addi", "subi", "andi", "ori", "nori", "bne", "beq", "blt", "bgt", "lb", "sb", "lw", "sw", "lh", "sh"
	}, *j[] = { /* J */
		"jmp", "la", "call", "stop"
	}, *instructions[] = { /* Instructors keywords. */
		"db", "dh", "dw", "asciz", "entry", "extern"
	};
	Expectation expectations[] = { /* Expectations for every instructor matching by index. */
		Expect8BitParams, Expect16BitParams, Expect32BitParams, ExpectString, ExpectLabel, ExpectLabel
	};

	/*
	 * For all following insertions if one fails no additional insertions
	 * will be made.
	 * Operators are inserted to the operators array and instructors to
	 * the instructors array.
	 */

	/* --------------------- operators ------------------------- */

	/* Inserts all R type keywords using a funct code pattern. */
	for (index = 0; index < breakR && code == SUCCESS; index++)
		code = insertOperator(r[index], R, (index % FUNCT_COUNT) + 1, 0, operatorsIndex++);
	for (index = breakR; index < rCount && code == SUCCESS; index++)
		code = insertOperator(r[index], R, (index % FUNCT_COUNT) + 1, 1, operatorsIndex++);

	/* Inserts all I type keywords using an opcode pattern. */
	for (index = 0; index < iCount && code == SUCCESS; index++)
		code = insertOperator(i[index], I, NO_FUNCT, index + iopcodeStart, operatorsIndex++);

	/* Inserts all J type keywords using an opcode pattern. */
	for (index = 0; index < (jCount - 1) && code == SUCCESS; index++)
		code = insertOperator(j[index], J, NO_FUNCT, index + jopcodeStart, operatorsIndex++);
	/* Inserts the "stop" keyword. */
	if (code == SUCCESS)
		code = insertOperator(j[jCount - 1], J, NO_FUNCT, stopopcode, operatorsIndex++);

	/* -------------------- instructors ------------------------ */

	/* Inserts the instructors keywords and expectations by index. */
	for (index = 0; index < INS_COUNT && code == SUCCESS; index++)
		code = insertInstructor(instructions[index], expectations[index], index);

	return code; /* Returning the code after all the insertions. */
}

/**
 * Frees all the the memory used by the data structures of this
 * translation unit.
 */
void clearasmKeywords() {
	int index;

	/* Freeing the memory used by the operators array */
	for (index = 0; index < OP_COUNT; index++)
		free(operators[index]);
	/* Freeing the memory used by the instructors array */
	for (index = 0; index < INS_COUNT; index++)
		free(instructors[index]);
}

void printasm() {
	int i;
	Operator *temp;
	for (i = 0; i < OP_COUNT; i++) {
		printf("[%d]:\n", i);
		temp = operators[i];
		printf("\t%s: type = %d, funct = %d, opcode = %d\n", temp->keyword, temp->type, temp->funct, temp->opcode);
	}
	printf("\n");
	for (i = 0; i < INS_COUNT; i++){
		printf("[%d]:\n", i);
		printf("\t%s: expecting = %d\n", instructors[i]->keyword, instructors[i]->expecting);
	}
	printf("\n");
}
