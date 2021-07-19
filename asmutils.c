#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "asmutils.h"
#include "utils.h"

/*
 * Contains a collection of utility functions for this assembler.
 * These functions are relevant for reading from the source file.
 */

#define FILE_EXTENSION ".as" /* The extension for the assembly source files. */
#define FILE_EXTENSION_LEN 3 /* The length of the assembly source file extension. */
/* Syntax characters */
#define NEW_LINE '\n'
#define COMMENT ';'
#define DOLLAR_SIGN '$'
#define COMMA ','
#define SPACE ' '
#define TAB '\t'
#define TERMINATING_CHAR '\0'

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
Flag extractSourceLine(FILE *sourceFile, char *line, int *lineLength) {
	/* The line can be longer than the defined limit but with spaces only. */
	Flag endStatus = WarningLineLengthFlag;
	char c; /* To store every character from the given stream. */
	int index; /* Number of characters in the current line. */

	/* Scans until the line length limit. */
	for (index = 0; index < SOURCE_LINE_LENGTH; index++) {
		c = fgetc(sourceFile);

		/* If the line ends before the limit: */
		if (c == NEW_LINE || c == EOF) {
			/* Inserting a terminating character */
			line[index] = TERMINATING_CHAR;
			/* There was no length related issue. */
			return NewLineFlag;
		}
		/* Adding every character before the limit to the buffer. */
		line[index] = c;
	}

	/* Setting the last place in the buffer to a terminating character. */
	line[index] = TERMINATING_CHAR;
	/* Checking if the SOURCE_LINE_LENGTH + 1 character was line ending. */
	c = fgetc(sourceFile);
	if (c == NEW_LINE || c == EOF)
		return NewLineFlag; /* There was no length related issue. */

	/* At this point the line is too long. */
	index++;
	/*
	 * Skipping the rest of the line while checking every character.
	 * If every character is a space then its a warning and the assembler
	 * can still create output files.
	 * If one character is not a space then it is an error and the
	 * assembler should not create output files.
	 */
	while (c != NEW_LINE && c != EOF) {
		if (c != SPACE || c != TAB)
			endStatus = ErrorLineLengthFlag;
		c = fgetc(sourceFile);
		index++;
	}
	/* 
	 * Returning the length of the line via a parameter.
	 * This will be used for the warning/error message.
	 */
	*lineLength = index;
	/* Can return a warning or an error flag. */
	return endStatus;
}

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
Flag rangeRParam(char *sourceLine, Expectation *expecting, const char expectedNumParam, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of the  */
	char c; /* For readability purposes. */
	char isSpaceAllowed = 1; /* To track parts were spaces or tabs can be. */
	char isLineEmpty = 1; /* To track if this part of the line is empty or not. */
	char paramCount = 0; /* Counts the number of seen dollar signs. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */
	*expecting = ExpectDollarSign; /* Expecting operands. */

	for (index = *startIndex; sourceLine[index] != TERMINATING_CHAR; index++) {
		c = sourceLine[index];
		if (c == TAB || c == SPACE) { /* Current character is space or tab. */
			if (!isSpaceAllowed) {
				endStatus = IllegalSpacingFlag;
				break; /* Found illegally positioned space or tab. */
			}
			/*
			 * Adjusting the range based on if the current position is before or
			 * after the operands.
			 */
			if (isLineEmpty)
				*startIndex = index + 1;
			else
				*endIndex = index - 1;
			if (*expecting == ExpectDigitOrEnd)
				break; /* Operands were read successfully. */
			if (*expecting == ExpectDigitOrComma)
				/* Digits cannot be separated by a space. */
				*expecting = ExpectComma;
			continue;
		}
		if (c == COMMENT) { /* Current character is a semicolon. */
			endStatus = StrayCommentFlag;
			break; /* A comment must have a dedicated line. */
		}
		if (c == DOLLAR_SIGN) { /* Current character is a dollar sign. */
			if (*expecting != ExpectDollarSign) {
				endStatus = StrayDollarSignFlag;
				break; /* Found illegally positioned dollar sign. */
			}
			isSpaceAllowed = 0; /* Cannot have a space between operand declaration and its value. */
			isLineEmpty = 0; /* The line is not empty. */
			paramCount++; /* Incrementing the number of seen operands. */
			*expecting = ExpectDigit; /* The next character should be a digit. */
			continue;
		}
		if (isdigit(c)) { /* Current character is a digit. */
			if (*expecting != ExpectDigit && *expecting != ExpectDigitOrComma && *expecting != ExpectDigitOrEnd) {
				endStatus = StrayDigitFlag;
				break; /* Found illegally positioned digit. */
			}
			/*
			 * The next character can be a digit or (based on the expected number of operands)
			 * a comma or a white character.
			 */
			*expecting = (paramCount < expectedNumParam) ? ExpectDigitOrComma : ExpectDigitOrEnd;
			*endIndex = index; /* In case end is expected and the next character is an end of line. */
			isSpaceAllowed = 1; /* Spaces are allowed here only if next non-white character is a comma. */
			continue;
		}
		if (c == COMMA) { /* Current character is a comma. */
			if (*expecting != ExpectDigitOrComma && *expecting != ExpectComma) {
				endStatus = StrayCommaFlag;
				break; /* Found illegally positioned comma. */
			}
			*expecting = ExpectDollarSign; /* The next character should be a dollar sign. */
			continue;
		}
		endStatus = UnexpectedFlag;
		break; /* Found an unexpected character. */
	}
	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || *expecting != ExpectDigitOrEnd)
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	return endStatus;
}

/**
 * Checks if the extension of the given file's name is an assembly source
 * code file, in other words if the given string ends with ".as".
 * Returns SUCCESS if the given file name is a valid assembly source code
 * name and ERROR otherwise.
 */
Code isValid(const char *fileName) {
	/* Number of characters in the given string. */
	int nameLength = strlen(fileName);
	/* To store the last characters in the given string (and \0). */
	char extension[FILE_EXTENSION_LEN + 1];

	/*
	 * Extracting the last characters from the given string and storing
	 * the result in the extension variable.
	 */
	subString(extension, fileName, nameLength - FILE_EXTENSION_LEN, nameLength);

	/*
	 * If the last 3 characters of the given string are equal to the assembly
	 * source code file name extension then a SUCCESS code would be returned.
	 * If the end of the given string is not the assembly source code file
	 * name extension then an ERROR code would be returned instead.
	 */
	return strcmp(extension, FILE_EXTENSION) == 0 ? SUCCESS : ERROR;

}
