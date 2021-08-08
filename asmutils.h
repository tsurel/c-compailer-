#ifndef ASMUTILS_H
#define ASMUTILS_H

#include <stdio.h>

/**
 * An header file for the assembler utilities (asmutils) translation unit.
 */

/* Error codes. */
#define ERROR 1 /* A return value that indicates a failed operation. */
#define SUCCESS 2 /* A return value that indicates a successful operation. */
/* Number of expected operands. */
#define R2 2 /* Number of expected operands for some R operators. */
#define R3 3 /* Number of expected operands for some R operators. */
/* Maximum source line length. */
#define SOURCE_LINE_LENGTH 80 /* Source line limit. */

/**
 * The error code data type can be ERROR or SUCCESS and is returned by
 * functions to determine if that function worked as intended or not.
 * For example: this code can be used to check if a memory allocation
 * by a function was successful or not.
 */
typedef unsigned char Code;

/**
 * The flag enumeration is used to distinguish different issues when
 * scanning assembly source files.
 */
typedef enum {
	NewLineFlag, /* For new line character handling. */
	CommentLineFlag, /* For comment line handling. */
	StrayDollarSignFlag, /* For an illegally positioned dollar sign. */
	StrayCommaFlag, /* For an illegally positioned dollar sign. */
	StrayDigitFlag, /* For an illegally positioned digit. */
	StrayCommentFlag, /* For an illegally positioned semicolon. */
	UnexpectedFlag, /* For unexpected character events. */
	IllegalSpacingFlag, /* For an illegally positioned spacing. */
	ErrorLineLengthFlag, /* For line length error handling. */
	NoIssueFlag, /* A default value. */
	WarningLineLengthFlag, /* For line length warning handling. */
    EndFileFlag, /* For detecting file endings. */
    IncompleteStringFlag, /* For cases where the string has only a begin quote. */
    HardwareErrorFlag /* For memory allocation issues. */
} Flag;

/**
 * The expectation enumeration is used to relate the previous part
 * of the syntax to the next one.
 */
typedef enum {
	ExpectEnd, /* The next part should be an ending. */
	ExpectDollarSign, /* The next character should be a dollar sign. */
	ExpectDigit, /* The next character should be a digit. */
	ExpectDigitOrComma, /* The next character should be a digit or a comma. */
	ExpectComma, /* The next character should be a comma. */
	ExpectDigitOrEnd, /* The next part should be a digit or an ending. */
    Expect8BitParams, /* The next part should be signed 8 bit parameters. */
    Expect16BitParams, /* The next part should be signed 16 bit parameters. */
    Expect32BitParams, /* The next part should be signed 32 bit parameters. */
    ExpectString, /* The next part should be a string declaration. */
    ExpectLabel, /* The next part should be a label symbol. */
    ExpectQuote
} Expectation;

/**
 * Scans from a given position of a stream until a new line or terminating
 * character and returns the first SOURCE_LINE_LENGTH characters (or less)
 * with an additional terminating character at the end.
 * Expects the second parameter to be a buffer SOURCE_LINE_LENGTH + 1 long.
 * This function also return a Flag in case there was an assembly syntax
 * related issue. If there was such issue the third parameter would hold
 * the length of the line.
 * Should return NewLineFlag flag if there was no issue.
 * This function checks for line length related issues only.
 * After calling this function the first parameter will be pointing to
 * the first character of the next line.
 */
Flag extractSourceLine(FILE *sourceFile, char *line, int *lineLength);

/**
 * Scans a portion from the given source line to find the range of indexes
 * between which the operands for R type operators are located, all while
 * checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue and the second
 * parameter should be set to ExpectDigitOrEnd with the two last parameters
 * set to the range of indexes between which the operands are coded (including).
 * In the case of a syntax error the returned flag and the second parameter
 * can be used to determine what it was while the last two parameters would
 * be equal to the index where the issue was found.
 * Expects the forth index to be set to the index from which the scan should
 * begin and the third index to be set to either 2 or 3.
 */
Flag rangeRParam(char *sourceLine, Expectation *expecting, const char expectedNumParam, int *startIndex, int *endIndex);

/**
 * Scans a portion from the given source line and extracts the string
 * definition that comes after an asciz keyword if there are no syntax
 * errors.
 * In case of a syntax error it can be deciphered using the returned flag
 * and the second parameter, also the last parameter would not be modified
 * and the third parameter would point to the index where the issue was found.
 * In case there were no issues the last parameter would contain the extracted
 * string and the third parameter would point to the index after the string
 * definition.
 * Note that memory for the last parameter is allocated on the heap.
 */
Flag getAscizParam(char *sourceLine, Expectation *expecting, int *index, char **stringParam);

/**
 * Checks if the extension of the given file's name is an assembly source
 * code file, in other words if the given string ends with ".as".
 * Returns SUCCESS if the given file name is a valid assembly source code
 * name and ERROR otherwise.
 */
Code isValid(const char *fileName);

#endif
