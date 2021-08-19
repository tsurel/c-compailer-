#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "asmutils.h"

/**
 * An header file for the keywords container (keywords) translation unit.
 */

#define R 1 /* Operator code for type R. */
#define I 2 /* Operator code for type I. */
#define J 3 /* Operator code for type J. */

/**
 * Defining the operator data structure.
 * This structure is used by this assembler for storing, accessing, and
 * differentiating assembly operator keywords.
 */
typedef struct op Operator;

/**
 * Defining the instructor data structure.
 * This structure is used by this assembler for storing, accessing, and
 * differentiating assembly Instructor keywords.
 */
typedef struct ins Instructor;

/**
 * Returns the type code of the given operator pointer.
 */
unsigned char getType(Operator *operator);

/**
 * Returns the funct code of the given operator pointer.
 */
unsigned char getFunct(Operator *operator);

/**
 * Returns the opcode code of the given operator pointer.
 */
unsigned char getOpcode(Operator *operator);

/**
 * Returns the keyword of the given operator pointer as a string.
 */
char *getOperatorKeyword(Operator *operator);

/**
 * Returns the expectation of the given instructor pointer.
 */
Expectation getExpectation(Instructor *instructor);

/**
 * Returns the keyword of the given instructor pointer as a string.
 */
char *getInstructorKeyword(Instructor *instructor);

/**
 * Searches for the related operator object using its string representation.
 * Returns a pointer to an operation container node, if the given parameter is a
 * code-word, otherwise a null pointer.
 */
Operator *searchOperatorByString(const char *keyword);

/**
 * Searches for the related instructor object using its string representation.
 * Returns a pointer to the instructor object, if the given parameter is a valid
 * keyword, otherwise a null pointer.
 */
Instructor *searchInstructorByString(const char *keyword);

/**
 * Initializes all the assembly keywords so the assembler could link the
 * string representation of an operator to its related codes, that way it
 * can be assembled.
 * At the end of the process, this function will return a code that tells if the
 * initialization was successful or not.
 * This function should be called only once.
 */
Code initasmKeywords();

/**
 * Frees all the the memory used by the data structures of this
 * translation unit.
 */
void clearasmKeywords();

#endif
