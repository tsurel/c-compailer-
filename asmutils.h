#ifndef ASMUTILS_H
#define ASMUTILS_H

#include <stdio.h>

/**
 * An header file for the assembler utilities (asmutils) translation unit.
 */

/* Error codes. */
#define ERROR 1 /* A return value that indicates a failed operation. */
#define SUCCESS 2 /* A return value that indicates a successful operation. */
/* Flags */
#define NEW_LINE 3 /* Indicates that a function passed a new line character in a stream. */
#define NO_SPACE_AFTER_LABEL 4 /* Using this macro means an exception was found in a source file. */

/**
 * The error code data type can be ERROR or SUCCESS and is returned by
 * functions to determine if that function worked as intended or not.
 * For example: this code can be used to check if a memory allocation
 * by a function was successful or not.
 */
typedef unsigned char Code;

/**
 * The flag data type can be set to the defined flag macros above and is
 * used to distinguish different situations when scanning assembly
 * source files.
 */
typedef unsigned char Flag;

/**
 * A structure that can hold a sequence of characters, extracted from the
 * source file, and a value to determine what the word means in the source
 * file.
 */
typedef struct {
	char *word; /* The extracted sequence of characters. */
	unsigned char type;  /* A value that tells information about the word. */
} SourceWord;

Flag nextSourceWord(FILE *sourceFile, SourceWord *sourceWord);
Code isValid(const char *fileName);

#endif
