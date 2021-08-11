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
    StraySignFlag, /* For an illegally positioned plus or minus. */
	UnexpectedFlag, /* For unexpected character events. */
	IllegalSpacingFlag, /* For an illegally positioned spacing. */
	ErrorLineLengthFlag, /* For line length error handling. */
	NoIssueFlag, /* A default value. */
	WarningLineLengthFlag, /* For line length warning handling. */
    EndFileFlag, /* For detecting file endings. */
    IncompleteStringFlag, /* For cases where the string has only a begin quote. */
    InvalidRegisterFlag, /* For invalid registers. */
    SizeOverflowFlag, /* For cases where a value is too big to be assigned. */
    IllegalSymbolFlag, /* For invalid label symbols. */
    HardwareErrorFlag, /* For memory allocation issues. */
    InstructorFlag, /* For data instruction line handling. */
    OperatorFlag, /* For operation line handling. */
    LabelFlag /* For label handling. */
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
	ExpectAlphanum, /* The next character should be alphanumeric. */
    ExpectDigitOrSign, /* The next character should be a digit or a plus or a minus. */
    ExpectDigitOrSignOrDollarSign, /* The next character can be one of those in the enumeration's name. */
	ExpectComma, /* The next character should be a comma. */
	ExpectDigitOrEnd, /* The next part should be a digit or an ending. */
    Expect8BitParams, /* The next part should be signed 8 bit parameters. */
    Expect16BitParams, /* The next part should be signed 16 bit parameters. */
    Expect32BitParams, /* The next part should be signed 32 bit parameters. */
	ExpectWord, /* The next part should be the beginning of the source line. */ 
    ExpectString, /* The next part should be a string declaration. */
    ExpectLabel, /* The next part should be a label symbol. */
    ExpectQuote /* The next part should be a string definition. */
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
 * Scans a portion of the given source line and extracts the arguments
 * into the last parameter if, there were no syntax errors.
 * In case of a syntax error it can be extracted using the returned flag
 * and the second parameter, also the third parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last parameter would contain the extracted
 * arguments and the third parameter would point to the index with the
 * terminating character (the end of the line) while the forth parameter would
 * contain the number of extracted arguments.
 * Expects the third parameter to be positioned before the arguments and the
 * second parameter to be either one of the following expectations:
 * Expect8BitParams, Expect16BitParams, Expect32BitParams.
 * This information is used for the arguments size checking.
 */
Flag getDataParam(char *sourceLine, Expectation *expecting, int *index, int *count, void **args);

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
 * Scans a portion of the given source line and extracts the operands
 * into the last three parameters.
 * In case of a syntax error it can be deciphered using the returned flag
 * and the second parameter, also the forth parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last three parameters would contain the
 * extracted operands and the forth parameter would point to the index after
 * the operands definition.
 * Expects the forth parameter to be positioned before the operands and the
 * third parameter to be either 2 or 3, if it is 2 then rt would be untouched.
 */
Flag getRParam(char *sourceLine, Expectation *expecting, const char paramCount, int *index, char *rs, char *rt, char *rd);

/**
 * Scans a portion of the given source line and extracts the operands
 * into the last four parameters.
 * In case of a syntax error it can be deciphered using the returned flag
 * and the second parameter, also the third parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last four parameters would contain the
 * extracted operands and the third parameter would point to the index after
 * the operands definition.
 * Expects the forth parameter to be positioned before the operands.
 * This function may or may not allocate memory on the heap for the last
 * parameter based on what the operands are. If the last operand is a label
 * then memory would be allocated (if there were no errors), and if the
 * second operand is an immediate value then the last parameter would be
 * set to a null pointer.
 */
Flag getIParam(char *sourceLine, Expectation *expecting, int *index, char *rs, char *rt, short *immed, char **label);

/**
 * Scans a portion of the given source line and extracts the operand into
 * one of the last two parameters.
 * In case of a syntax error it can be deciphered using the returned flag
 * and the second parameter, also the third parameter would point to the index
 * where the issue was found.
 * In case there were no issues one of the last two parameters would contain the
 * extracted operand and the third parameter would point to the index after
 * the operands definition.
 * Expects the third parameter to be positioned before the operands.
 * This function may or may not allocate memory on the heap for the last
 * parameter based on what the operand is. If the operand is a label then
 * memory would be allocated (if there were no errors), and if the
 * operand is a register then the last parameter would be set to a
 * null pointer while the fourth parameter would point to the register's address.
 */
Flag getJParam(char *sourceLine, Expectation *expecting, int *index, char *reg, char **label);

/**
 * Scans a portion of the given source line and extracts the word
 * into the last parameter if there were no syntax errors.
 * In case of a syntax error, it can be extracted using the returned flag
 * and the second parameter, while the third would point to the index where
 * the issue was found.
 * In case there were no issues, the last parameter would contain the extracted
 * word, also the third parameter would point to the index after that word.
 * Expects the third parameter to point to the index before the word.
 * Note that this function also check for a comment line, in that case the
 * last parameter would remain untouched and the third parameter would point
 * to the index where the comment character is positioned in the line.
 */
Flag getWord(char *sourceLine, Expectation *expecting, int *index, char **word);

/**
 * Checks if the extension of the given file's name is an assembly source
 * code file, in other words if the given string ends with ".as".
 * Returns SUCCESS if the given file name is a valid assembly source code
 * name and ERROR otherwise.
 */


Code isValid(const char *fileName);

#endif
