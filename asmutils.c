#include <stdio.h>
#include <stdlib.h>
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
#define DECIMAL 10 /* Decimal base, used for conversion from text to integer. */
#define MAX_REGISTER 31 /* The highest register on the CPU. */
/* Syntax characters */
#define NEW_LINE '\n'
#define COMMENT ';'
#define DOLLAR_SIGN '$'
#define COMMA ','
#define SPACE ' '
#define TAB '\t'
#define QUOTE '\"'
#define TERMINATING_CHAR '\0'

/**
 * The following functions should not be used outside of this translation unit.
 */
Flag rangeRParam(char *sourceLine, Expectation *expecting, const char expectedNumParam, int *startIndex, int *endIndex);
Flag extractRParam(char *sourceLine, const int startIndex, const char paramCount, char *rs, char *rt, char *rd);
Flag rangeAscizParam(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex);
void extractAscizParam(char *sourceLine, const int startIndex, const int endIndex, char **stringParam);

/**
 * Scans from a given position of a stream until a new line or terminating
 * character and returns the first SOURCE_LINE_LENGTH characters (or less)
 * with an additional terminating character at the end.
 * Expects the second parameter to be a buffer SOURCE_LINE_LENGTH + 1 long.
 * This function also return a Flag in case there was an assembly syntax
 * related issue. If there was such issue the third parameter would hold
 * the length of the line.
 * Should return NewLineFlag flag or EndFileFlag flag if there was no issue.
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
			if (c == NEW_LINE)
				return NewLineFlag;
			else
				return EndFileFlag;
		}
		/* Adding every character before the limit to the buffer. */
		line[index] = c;
	}

	/* Setting the last place in the buffer to a terminating character. */
	line[index] = TERMINATING_CHAR;
	/* Checking if the SOURCE_LINE_LENGTH + 1 character was line ending. */
	c = fgetc(sourceFile);

	/* There was no length related issue. */
	if (c == NEW_LINE)
		return NewLineFlag;
	else if (c == EOF)
		return EndFileFlag;

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
		if (c != SPACE && c != TAB)
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
 * between which the string for the asciz keyword is located, all while
 * checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue and the second
 * parameter should be set to ExpectEnd with the two last parameters set
 * to the range of indexes between which the string is coded (including).
 * In the case of a syntax error the returned flag and the second parameter
 * can be used to determine what it was while the last two parameters would
 * be equal to the index where the issue was found.
 * Expects the third parameter to be set to the index from which the scan
 * should begin.
 */
Flag rangeAscizParam(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
	char c; /* For readability purposes. */
	char isLineEmpty = 1; /* To track if this part of the line is empty or not. */
	char incompleteString = 0; /* To track if the string has a second quotation mark or if it has one at all. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */
	*expecting = ExpectQuote; /* Expecting a quotation mark. */

	for (index = *startIndex; sourceLine[index] != TERMINATING_CHAR; index++) {
		c = sourceLine[index];

		if (c == TAB || c == SPACE) { /* Current character is space or tab. */
			/*
			 * Setting the beginning of the range based on if the current position
			 * is before the string definition.
			 */
			if (isLineEmpty)
				*startIndex = index + 1;

			if (*expecting == ExpectEnd)
				break; /* String was scanned successfully. */
			continue;
		}
		if (c == COMMENT) { /* Current character is a semicolon. */
			endStatus = StrayCommentFlag;
			break; /* A comment must have a dedicated line. */
		}
		if (c == QUOTE) { /* Current character is a quotation mark. */
			if (isLineEmpty) {
				isLineEmpty = 0; /* The string definition has begun (first quote). */
				incompleteString = 1;
			} else {
				*expecting = ExpectEnd; /* Second quote. */
				incompleteString = 0;
			}
			continue;
		}
		if (*expecting == ExpectQuote) {
			if (isLineEmpty) {
				/* Unexpected characters have appeared outside the string. */
				endStatus = UnexpectedFlag;
				break;
			}
			continue; /* Characters are within the string. */
		}
		if (*expecting == ExpectEnd) {
			/* Unexpected characters have appeared outside the string. */
			endStatus = UnexpectedFlag;
			break;
		}
	}

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || *expecting != ExpectEnd)
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	/* Differentiating between no string and an incomplete string. */
	if (incompleteString)
		endStatus = IncompleteStringFlag;

	return endStatus;
}

/**
 * Uses the given range to extract the asciz parameter (string) that is within
 * that range into the last parameter. The returned value on the last parameter
 * is the string without the quotation marks on the edges and a terminating
 * character at the end.
 * This function allocates memory on the heap for the last parameter, in
 * case the allocation failed the last parameter would be null.
 */
void extractAscizParam(char *sourceLine, const int startIndex, const int endIndex, char **stringParam) {
	int index; /* Index on the line (source). */
	int paramIndex = 0; /* Index on the string parameter (destination). */
	char *temp; /* Temporary array to copy the string to. */

	temp = malloc(endIndex - startIndex); /* Allocating memory for the string. */
	if (temp == NULL)
		return; /* Cannot continue without memory. */

	/* The following loop copies the string from the given line to the last parameter. */
	for (index = startIndex + 1; index < endIndex; index++, paramIndex++){
		temp[paramIndex] = sourceLine[index];
	}

	/* Adding the terminating character at the end. */
	temp[paramIndex] = TERMINATING_CHAR;
	*stringParam = temp; /* Setting the last parameter to point to that array. */
}

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
Flag getAscizParam(char *sourceLine, Expectation *expecting, int *index, char **stringParam) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source file. */

	/* Getting the range of indexes between which the string is defined. */
	endStatus = rangeAscizParam(sourceLine, expecting, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the character after the string. */

	/* If no syntax issues were found the string would be extracted. */
	if (endStatus == NoIssueFlag && *expecting == ExpectEnd) {
		extractAscizParam(sourceLine, startIndex, endIndex, stringParam);
		if (*stringParam == NULL)
			endStatus = HardwareErrorFlag; /* Memory allocation was not successful. */
	} else
		/* If there is a syntax error then the index would be set to that position. */
		*index = endIndex;

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
 * Expects the forth parameter to be set to the index from which the scan should
 * begin and the third index to be set to either 2 or 3.
 */
Flag rangeRParam(char *sourceLine, Expectation *expecting, const char expectedNumParam, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
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
			 * Setting the beginning of the range based on if the current position
			 * is before the operands.
			 */
			if (isLineEmpty)
				*startIndex = index + 1;

			if (*expecting == ExpectDigitOrEnd)
				break; /* Operands were scanned successfully. */
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

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || *expecting != ExpectDigitOrEnd)
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	return endStatus;
}

/**
 * Uses the given start index to scan for operands from the given line
 * and returns them through the last three parameters.
 * The third parameter can be either 2 or 3, if it is set to 2 rt would
 * be ignored and would not be set.
 * This function also check if the register addresses are valid, if they
 * are not then InvalidRegisterFlag flag would be returned.
 * Expects the second parameter to be set to the index where the R
 * parameters definition begins.
 */
Flag extractRParam(char *sourceLine, const int startIndex, const char paramCount, char *rs, char *rt, char *rd) {
	/* A pointer to track the position on the line. */
	char *pointer = sourceLine + startIndex + 1; /* That pointer is immediately set to the first digit of the first operand. */

	*rs = strtol(pointer, &pointer, DECIMAL); /* Extracting the first operand. */
	/* Checking if the operand is valid. */
	if (*rs < 0 || *rs > MAX_REGISTER)
		return InvalidRegisterFlag; /* The operand is not valid. */
	/* Skipping to the next operand. */
	while (!isdigit(*pointer))
		pointer++;

	/* This register should be set only if paramCount is set to 3. */
	if(paramCount == R3) {
		*rt = strtol(pointer, &pointer, DECIMAL); /* Extracting the second operand. */
		/* Checking if the operand is valid. */
		if (*rt < 0 || *rt > MAX_REGISTER)
			return InvalidRegisterFlag; /* The operand is not valid. */
		/* Skipping to the next operand. */
		while (!isdigit(*pointer))
			pointer++;
	}

	*rd = strtol(pointer, &pointer, DECIMAL); /* Extracting the last operand. */
	/* Checking if the operand is valid. */
	if (*rd < 0 || *rd > MAX_REGISTER)
		return InvalidRegisterFlag; /* The operand is not valid. */

	return NoIssueFlag; /* The extraction was completed successfully. */
}

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
Flag getRParam(char *sourceLine, Expectation *expecting, const char paramCount, int *index, char *rs, char *rt, char *rd) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source file. */

	/* Getting the range of indexes between which the operands are defined. */
	endStatus = rangeRParam(sourceLine, expecting, paramCount, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the character after the operands. */

	/* If no syntax issues were found the string would be extracted. */
	if (endStatus == NoIssueFlag && *expecting == ExpectDigitOrEnd)
		endStatus = extractRParam(sourceLine, startIndex, paramCount, rs, rt, rd);
	else
		/* If there is a syntax error then the index would be set to that position. */
		*index = endIndex;

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
