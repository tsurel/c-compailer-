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
#define BYTE_SIZE 1 /* Arguments size for the db instructor. */
#define HALF_SIZE 2 /* Arguments size for the dh instructor. */
#define WORD_SIZE 4 /* Arguments size for the dw instructor. */
#define MAX_SIGNED_BYTE 127 /* Maximum value for a signed byte. */
#define MIN_SIGNED_BYTE -128 /* Minimum value for a signed byte. */
#define MAX_SIGNED_HALF 32767 /* Maximum value for a signed half word. */
#define MIN_SIGNED_HALF -32768 /* Minimum value for a signed half word. */
#define MAX_SIGNED_WORD 2147483647 /* Maximum value for a signed word. */
#define MIN_SIGNED_WORD -2147483648 /* Minimum value for a signed word. */
#define MAX_LABEL_LENGTH 31 /* the maximum allowed length for first word in a row */

/* Syntax characters */
#define NEW_LINE '\n'
#define COMMENT ';'
#define DOLLAR_SIGN '$'
#define COMMA ','
#define SPACE ' '
#define TAB '\t'
#define QUOTE '\"'
#define PLUS '+'
#define MINUS '-'
#define TERMINATING_CHAR '\0'

/**
 * The following functions should not be used outside of this translation unit.
 */
Flag rangeDataParam(char *sourceLine, Expectation *expecting, int *count, int *startIndex, int *endIndex);
Flag extractDataParam(char *sourceLine, const Expectation expecting, const int count, const int startIndex, void **args);

Flag rangeAscizParam(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex);
void extractAscizParam(char *sourceLine, const int startIndex, const int endIndex, char **stringParam);

Flag rangeRParam(char *sourceLine, Expectation *expecting, const char expectedNumParam, int *startIndex, int *endIndex);
Flag extractRParam(char *sourceLine, const int startIndex, const char paramCount, char *rs, char *rt, char *rd);

Flag rangeIParam(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex);
Flag extractIParam(char *sourceLine, const int startIndex, const int endIndex, char *rs, char *rt, short *immed, char **label);

Flag rangeJParam(char *sourceLine, Expectation *expecting, char *isRegister, int *startIndex, int *endIndex);
Flag extractJParam(char *sourceLine, const int startIndex, const int endIndex, const char isRegister, char *reg, char **label);

Flag extractWord(char *sourceLine, const Expectation expecting, const int startIndex, const int endIndex, void **args);
Flag rangeWord(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex);
Flag getWord(char *sourceLine, Expectation *expecting, int *index, void **args);


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
 * between which the argument for db, dh, or dw instructors are located,
 * all while checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue and the second
 * parameter should be set to ExpectEnd with the two last parameters set
 * to the range of indexes between which the arguments are coded (including).
 * Note that the arguments end at the last character of the line, even if
 * there are spaces between the actual arguments and the end of the line.
 * In the case of a syntax error the returned flag and the second parameter
 * can be used to determine what it was while the last two parameters would
 * be equal to the index where the issue was found.
 * Expects the fourth parameter to be set to the index from which the scan should
 * begin.
 * The third parameter, if there were no issues, would be set to the number of
 * arguments found in this line.
 */
Flag rangeDataParam(char *sourceLine, Expectation *expecting, int *count, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
	char c; /* For readability purposes. */
	char isLineEmpty = 1; /* To track if this part of the line is empty or not. */
	char isSpaceAllowed = 1; /* To track parts were spaces or tabs can be. */
	char canCount = 1; /* To track counting the number of arguments. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */
	*expecting = ExpectDigitOrSign; /* Expecting a digit or a plus sign or a minus sign. */

	*count = 0; /* Starting from zero. */

	for (index = *startIndex; sourceLine[index] != TERMINATING_CHAR; index++) {
		c = sourceLine[index];

		if (c == TAB || c == SPACE) { /* Current character is space or tab. */
			/*
			 * Setting the beginning of the range based on if the current position
			 * is before the parameters definition.
			 */
			if (isLineEmpty)
				*startIndex = index + 1;

			if (!isSpaceAllowed) {
				endStatus = IllegalSpacingFlag;
				break; /* Found an illegally positioned space or tab. */
			}

			if (*expecting == ExpectDigitOrComma)
				*expecting = ExpectComma; /* There cannot be a space between digits. */

			continue;
		}
		if (c == COMMENT) { /* Current character is a semicolon. */
			endStatus = StrayCommentFlag;
			break; /* A comment must have a dedicated line. */
		}
		if (c == PLUS || c == MINUS) { /* Current character is a plus or a minus. */
			if (*expecting != ExpectDigitOrSign) {
				endStatus = StraySignFlag;
				break; /* Found an illegally positioned plus or minus. */
			}
			isLineEmpty = 0; /* The line is not empty. */
			isSpaceAllowed = 0; /* There cannot be a space between a sign and a digit. */
			*expecting = ExpectDigit; /* The next character should be a digit. */
			if (canCount) { /* Counting the number of arguments on this line. */
				canCount = 0;
				(*count)++;
			}
			continue;
		}
		if (isdigit(c)) { /* Current character is a digit. */
			if (*expecting != ExpectDigit && *expecting != ExpectDigitOrComma && *expecting != ExpectDigitOrSign) {
				endStatus = StrayDigitFlag;
				break; /* Found an illegally positioned digit. */
			}
			isSpaceAllowed = 1; /* There can be spaces after digits. */
			isLineEmpty = 0;  /* The line is not empty. */
			*expecting = ExpectDigitOrComma; /* The next character can be a digit or a comma. */
			if (canCount) { /* Counting the number of arguments on this line. */
				canCount = 0;
				(*count)++;
			}
			continue;
		}
		if (c == COMMA) { /* Current character is a comma. */
			if (*expecting != ExpectComma && *expecting != ExpectDigitOrComma) {
				endStatus = StrayCommaFlag;
				break; /* Found an illegally positioned comma. */
			}
			*expecting = ExpectDigitOrSign; /* Expecting another argument after a comma. */
			canCount = 1; /* Another argument should come after a comma. */
			continue;
		}
		endStatus = UnexpectedFlag;
		break; /* Found an unexpected character. */
	}

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || (*expecting != ExpectComma && *expecting != ExpectDigitOrComma))
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	else
		*expecting = ExpectEnd; /* Making checking for issues easier outside this function. */

	return endStatus;
}

/**
 * Uses the given start index to scan for arguments from the given line
 * and returns them through the last parameter, while checking for
 * sizes issues.
 * if there are no issues the return flag should be NoIssueFlag flag.
 * Expects the second parameter to be one of the following expectations:
 * Expect8BitParams, Expect16BitParams, Expect32BitParams.
 * This information is used for the arguments size checking.
 * The fourth parameter should be pointing to the position before the
 * arguments start.
 * The third parameter should contain the number of arguments on this line.
 * This function allocates memory on the heap for the last parameter.
 */
Flag extractDataParam(char *sourceLine, const Expectation expecting, const int count, const int startIndex, void **args) {
	int argsIndex = 0; /* To track where to place each argument on the temporary array. */
	void *check; /* Used for checking if the memory allocation was successful. */
	char *dbTemp; /* A temporary array for db arguments. */
	short *dhTemp; /* A temporary array for dh arguments. */
	long *dwTemp; /* A temporary array for dw arguments. */

	long int value; /* Used for checking the size of the argument. */
	/* A pointer to track the position on the line. */
	char *pointer = sourceLine + startIndex; /* That pointer is immediately set to the first digit of the first argument. */
	Flag endStatus = NoIssueFlag; /* To check for issues. */

	/* Allocating memory for the array of arguments. */
	if (expecting == Expect8BitParams)
		check = dbTemp = calloc(count, BYTE_SIZE); /* db arguments. */
	else if (expecting == Expect16BitParams)
		check = dhTemp = calloc(count, HALF_SIZE); /* dh arguments. */
	else if (expecting == Expect32BitParams)
		check = dwTemp = calloc(count, WORD_SIZE); /* dw arguments. */

	if (check == NULL)
		return HardwareErrorFlag; /* Cannot continue without memory. */

	/* Lopping through all the arguments on this line. */
	while (argsIndex < count) {
		/* If a number if found it would be extracted. */
		if (isdigit(*pointer) || *pointer == PLUS || *pointer == MINUS) {
			value = strtol(pointer, &pointer, DECIMAL); /* Extracting the argument. */

			/* Checking the size of the argument. */
			if (expecting == Expect8BitParams) {
				if (value < MIN_SIGNED_BYTE || value > MAX_SIGNED_BYTE)
					endStatus = SizeOverflowFlag; /* db overflow. */
				dbTemp[argsIndex++] = value; /* Inserting the argument into the temporary db array. */
			} else if (expecting == Expect16BitParams) {
				if (value < MIN_SIGNED_HALF || value > MAX_SIGNED_HALF)
					endStatus = SizeOverflowFlag; /* dh overflow. */
				dhTemp[argsIndex++] = value; /* Inserting the argument into the temporary dh array. */
			} else if (expecting == Expect32BitParams) {
				if (value < MIN_SIGNED_WORD || value > MAX_SIGNED_WORD)
					endStatus = SizeOverflowFlag; /* dw overflow. */
				dwTemp[argsIndex++] = value; /* Inserting the argument into the temporary dw array. */
			}
			/* In case of an overflow the less important bytes are taken. */
		}
		/* Skipping the rest of the characters that are not a part of a number. */
		pointer++;
	}

	*args = check; /* Assigning the address of the array to the return parameter. */
	return endStatus;
}

/**
 * Scans a portion of the given source line and extracts the arguments
 * into the last parameter if, there were no syntax errors.
 * In case of a syntax error it can be extracted using the returned flag
 * and the second parameter, also the third parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last parameter would contain the extracted
 * arguments and the third parameter would point to the index with the
 * terminating character (the end of the line) while the fourth parameter would
 * contain the number of extracted arguments.
 * Expects the third parameter to be positioned before the arguments and the
 * second parameter to be either one of the following expectations:
 * Expect8BitParams, Expect16BitParams, Expect32BitParams.
 * This information is used for the arguments size checking.
 */
Flag getDataParam(char *sourceLine, Expectation *expecting, int *index, int *count, void **args) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source line. */
	Expectation sizeExpectation = *expecting;

	*count = 0; /* Initializing the argument counting variable. */
	/* Getting the range of indexes between which the operands are defined. */
	endStatus = rangeDataParam(sourceLine, expecting, count, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the position of the terminating character. */

	/* If no syntax issues were found the arguments would be extracted. */
	if (endStatus == NoIssueFlag && *expecting == ExpectEnd)
		endStatus = extractDataParam(sourceLine, sizeExpectation, *count, startIndex, args);
	else
		/* If there is a syntax error then the index would be set to its position. */
		*index = endIndex;
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

			continue;
		}
		if (c == QUOTE) { /* Current character is a quotation mark. */
			if (incompleteString)
				*expecting = ExpectEnd; /* This might be an ending quote. */
			else
				*expecting = ExpectQuote; /* This is not an ending quote. */

			isLineEmpty = 0; /* This part of the line is not empty. */
			/* quotes can appear within the string but there has to be one on each edge. */
			incompleteString = !incompleteString;
			*endIndex = index; /* Setting\resetting the end of the range. */
			continue;
		}
		if (isLineEmpty) {
			/* Unexpected characters have appeared outside the string. */
			endStatus = UnexpectedFlag;
			break;
		}
		*expecting = ExpectQuote; /* If any other character had appeared then the string should not end. */
		
	}

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

	/* The following loop copies the string from the given line to the temporary array. */
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
 * Expects the fourth parameter to be set to the index from which the scan should
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
 * operands definition begins.
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
 * and the second parameter, also the fourth parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last three parameters would contain the
 * extracted operands and the fourth parameter would point to the index after
 * the operands definition.
 * Expects the fourth parameter to be positioned before the operands and the
 * third parameter to be either 2 or 3, if it is 2 then rt would be untouched.
 */
Flag getRParam(char *sourceLine, Expectation *expecting, const char paramCount, int *index, char *rs, char *rt, char *rd) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source line. */

	/* Getting the range of indexes between which the operands are defined. */
	endStatus = rangeRParam(sourceLine, expecting, paramCount, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the character after the operands. */

	/* If no syntax issues were found the operand would be extracted. */
	if (endStatus == NoIssueFlag && *expecting == ExpectDigitOrEnd)
		endStatus = extractRParam(sourceLine, startIndex, paramCount, rs, rt, rd);
	else
		/* If there is a syntax error then the index would be set to that position. */
		*index = endIndex;

	return endStatus;
}

/**
 * Scans a portion from the given source line to find the range of indexes
 * between which the operands for I type operators are located, all while
 * checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue and the second
 * parameter should be set to ExpectDigitOrEnd or ExpectLabel with the two last
 * parameters set to the range of indexes between which the operands are coded
 * (including).
 * In the case of a syntax error the returned flag and the second parameter
 * can be used to determine what it was while the last two parameters would
 * be equal to the index where the issue was found.
 * Expects the third parameter to be set to the index from which the scan should
 * begin.
 */
Flag rangeIParam(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
	char c; /* For readability purposes. */
	char isSpaceAllowed = 1; /* To track parts were spaces or tabs can be. */
	char isLineEmpty = 1; /* To track if this part of the line is empty or not. */
	char isExpectingLabel = 0; /* To track if the last operand should be a label. */
	char isExpectingRegister = 0; /* To track if the last operand should be a register. */
	char isOnLabel = 0; /* To track when to stop after a label */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */
	*expecting = ExpectDollarSign; /* Expecting register operand. */

	for (index = *startIndex; sourceLine[index] != TERMINATING_CHAR; index++) {
		c = sourceLine[index];
		if (c == SPACE || c == TAB) { /* Current character is space or tab. */
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
			if (isOnLabel)
				break; /* Operands were scanned successfully with a label. */
			if (*expecting == ExpectDigitOrComma)
				*expecting = ExpectComma; /* Digits cannot be separated by a space. */

			continue;
		}
		if (c == COMMENT) { /* Current character is a semicolon. */
			endStatus = StrayCommentFlag;
			break; /* A comment must have a dedicated line. */
		}
		if (c == DOLLAR_SIGN) { /* Current character is a dollar sign. */
			if (*expecting != ExpectDollarSign && *expecting != ExpectDigitOrSignOrDollarSign) {
				endStatus = StrayDollarSignFlag;
				break; /* Found illegally positioned dollar sign. */
			}
			if (!isLineEmpty) {
				isExpectingLabel = 1; /* If the second operand is a register then the last is a label. */
			}
			isSpaceAllowed = 0; /* Cannot have a space between operand declaration and its value. */
			isLineEmpty = 0; /* The line is not empty. */
			*expecting = ExpectDigit; /* The next character should be a digit. */
			continue;
		}
		if (c == PLUS || c == MINUS) { /* Current character is a plus or a minus. */
			if (*expecting != ExpectDigitOrSignOrDollarSign) {
				endStatus = StraySignFlag;
				break; /* Found an illegally positioned plus or minus. */
			}
			isExpectingRegister = 1; /* The last operand should be a register */
			isSpaceAllowed = 0; /* There cannot be a space between a sign and a digit. */
			*expecting = ExpectDigit; /* The next character should be a digit. */
			continue;
		}
		if (isdigit(c)) { /* Current character is a digit. */
			if (*expecting != ExpectDigit && *expecting != ExpectDigitOrComma && *expecting != ExpectDigitOrSignOrDollarSign && *expecting != ExpectLabel && *expecting != ExpectDigitOrEnd) {
				endStatus = StrayDigitFlag;
				break; /* Found illegally positioned digit. */
			}
			if (*expecting == ExpectLabel){
				isOnLabel = 1; /* The label has started. */
				continue; /* A digit can be a part of a label. */
			}
			if (*expecting == ExpectDigitOrSignOrDollarSign)
				isExpectingRegister = 1; /* The second operand may not start with a plus or a minus. */

			/* If both variables are set to one then that has to be the last operand and, at this point it can only be a register. */
			*expecting = (isExpectingLabel && isExpectingRegister) ? ExpectDigitOrEnd : ExpectDigitOrComma; /* The next non-white character should be a digit or a comma */
			isSpaceAllowed = 1; /* Spaces are allowed here only if next non-white character is a comma. */
			continue;
		}
		if (c == COMMA) { /* Current character is a comma. */
			if (*expecting != ExpectDigitOrComma && *expecting != ExpectComma) {
				endStatus = StrayCommaFlag;
				break; /* Found illegally positioned comma. */
			}
			/* The next operand should be a register or an immediate value, or a label. */
			if (isExpectingLabel)
				*expecting = ExpectLabel; /* The last operand is a label. */
			else if (isExpectingRegister)
				*expecting = ExpectDollarSign; /* The last operand is a register. */
			else
				*expecting = ExpectDigitOrSignOrDollarSign; /* The next operand is the second operand. */
			continue;
		}
		if (isalpha(c)) {
			if (*expecting != ExpectLabel) {
				endStatus = UnexpectedFlag;
				break; /* Found an unexpected token; */
			}
			isOnLabel = 1; /* The label has started. */
			continue;
		}
		endStatus = UnexpectedFlag;
		break; /* Found an unexpected token, that is any other character. */
	}

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || (*expecting != ExpectDigitOrEnd && *expecting != ExpectLabel))
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	return endStatus;
}

/**
 * Uses the given range to scan for operands from the given line and
 * returns them through the last four parameters. Only one of the
 * last two would contain actual value.
 * If the last operand is a label then immed would remain untouched,
 * and if the second operand is an immediate value then the last
 * parameter would be a null pointer. rt and rs will store the
 * two other operands.
 * This function may or may not allocate memory on the heap for the last
 * parameter.
 * This function also check if the register addresses are valid, if they
 * are not then InvalidRegisterFlag flag would be returned and no memory
 * would be allocated, same for the length of the label.
 * Expects the second parameter to be set to the index where the I
 * operands definition begins.
 */
Flag extractIParam(char *sourceLine, const int startIndex, const int endIndex, char *rs, char *rt, short *immed, char **label) {
	int destIndex; /* To track the temporary array of the symbol */
	int symbolSize; /* To store the length of the symbol array. */
	char isLabel = 0; /* To track what are the operands to extract. */
	long int value = 0; /* To store every extracted value to check limits before assignment. */
	/* A pointer to track the position on the line. */
	char *pointer = sourceLine + startIndex + 1; /* That pointer is immediately set to the first digit of the first operand. */
	char *temp = NULL; /* A temporary pointer to point to the symbol of the label if there is one. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source line. */

	value = strtol(pointer, &pointer, DECIMAL); /* Extracting the first operand. */
	/* Checking if the operand is valid. */
	if (value < 0 || value > MAX_REGISTER)
		return InvalidRegisterFlag; /* The operand is not valid. */
	*rs = value; /* The operand is valid. */

	/* Skipping to the next operand. */
	while (!isdigit(*pointer) && *pointer != PLUS && *pointer != MINUS) {
		if (*pointer == DOLLAR_SIGN) { /* The second operand is a register. */
			isLabel = 1; /* The last operand must be a label. */
			pointer++;
			break; /* The next character must be a digit */
		}
		pointer++;
	}

	if (isLabel) { /* Handling the label with the other operand. */
		value = strtol(pointer, &pointer, DECIMAL); /* Extracting the second operand. */
		/* Checking if the operand is valid. */
		if (value < 0 || value > MAX_REGISTER)
			return InvalidRegisterFlag; /* The operand is not valid. */
		*rt = value; /* The operand is valid. */
		
		/* Skipping to the label. */
		while (!isalnum(*pointer))
			pointer++;
		if (isdigit(*pointer))
			return IllegalSymbolFlag; /* Label symbols cannot start with a digit. */
		/* Allocating memory for the label's symbol. */
		symbolSize = endIndex - (pointer - sourceLine) + 2;
		if (symbolSize > MAX_REGISTER + 1) /* + 1 for a terminating character. */
			return IllegalSymbolFlag; /* Label symbols cannot be longer than 31. */
		if ((temp = malloc(symbolSize)) == NULL)
			return HardwareErrorFlag; /* Cannot continue without memory. */

		/* The following loop copies the symbol from the given line to the temporary array. */
		for (destIndex = 0; destIndex < symbolSize - 1; destIndex++, pointer++){
			temp[destIndex] = *pointer;
		}
		/* Adding the terminating character at the end. */
		temp[destIndex] = TERMINATING_CHAR;
	} else { /* Handling the immediate value with the other operand. */
		value = strtol(pointer, &pointer, DECIMAL); /* Extracting the second operand. */
		/* Checking if the operand has a valid size. */
		if (value < MIN_SIGNED_HALF || value > MAX_SIGNED_HALF)
			endStatus = SizeOverflowFlag; /* The immediate value has overflowed 2 bytes. */
		/* In case of an overflow the less important bytes are taken. */

		*immed = value; /* Assigning the immediate value. */

		/* Skipping to the next operand. */
		while (!isdigit(*pointer))
			pointer++;
		value = strtol(pointer, &pointer, DECIMAL); /* Extracting the last operand. */
		/* Checking if the operand is valid. */
		if (value < 0 || value > MAX_REGISTER)
			return InvalidRegisterFlag; /* The operand is not valid. */
		*rt = value; /* The operand is valid. */
	}
	*label = temp; /* Assigning the symbol to the return parameter, if there was one. Null if otherwise. */
	return endStatus;
}

/**
 * Scans a portion of the given source line and extracts the operands
 * into the last four parameters.
 * In case of a syntax error it can be deciphered using the returned flag
 * and the second parameter, also the third parameter would point to the index
 * where the issue was found.
 * In case there were no issues the last four parameters would contain the
 * extracted operands and the third parameter would point to the index after
 * the operands definition.
 * Expects the fourth parameter to be positioned before the operands.
 * This function may or may not allocate memory on the heap for the last
 * parameter based on what the operands are. If the last operand is a label
 * then memory would be allocated (if there were no errors), and if the
 * second operand is an immediate value then the last parameter would be
 * set to a null pointer.
 */
Flag getIParam(char *sourceLine, Expectation *expecting, int *index, char *rs, char *rt, short *immed, char **label) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source line. */

	/* Getting the range of indexes between which the operands are defined. */
	endStatus = rangeIParam(sourceLine, expecting, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the character after the operands. */

	/* If no syntax issues were found the operand would be extracted. */
	if (endStatus == NoIssueFlag && (*expecting == ExpectDigitOrEnd || *expecting == ExpectLabel))
		endStatus = extractIParam(sourceLine, startIndex, endIndex, rs, rt, immed, label);
	else
		/* If there is a syntax error then the index would be set to that position. */
		*index = endIndex;

	return endStatus;
}

/**
 * Scans a portion from the given source line to find the range of indexes
 * between which the operands for J type operators is located, all while
 * checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue and the second
 * parameter should be set to ExpectDigitOrEnd or ExpectLabel with the two last
 * parameters set to the range of indexes between which the operands are coded
 * (including).
 * In the case of a syntax error the returned flag and the second parameter
 * can be used to determine what it was while the last two parameters would
 * be equal to the index where the issue was found.
 * Expects the fourth parameter to be set to the index from which the scan should
 * begin.
 */
Flag rangeJParam(char *sourceLine, Expectation *expecting, char *isRegister, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
	char c; /* For readability purposes. */
	char isSpaceAllowed = 1; /* To track parts were spaces or tabs can be. */
	char isLineEmpty = 1; /* To track if this part of the line is empty or not. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */

	*expecting = ExpectDollarSign; /* Expecting the beginning of the operand, but it is not necessarily a register. */
	*isRegister = 0; /* The operand is yet to be scanned. */

	for (index = *startIndex; sourceLine[index] != TERMINATING_CHAR; index++) {
		c = sourceLine[index];
		if (c == SPACE || c == TAB) { /* Current character is space or tab. */
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

			if (*expecting == ExpectDigitOrEnd || *expecting == ExpectLabel)
				break; /* Operand was scanned successfully. */
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
			*isRegister = 1; /* The operand is a register. */
			*expecting = ExpectDigit; /* The next character should be a digit. */
			continue;
		}
		if (isdigit(c)) { /* Current character is a digit. */
			if (*expecting != ExpectDigit && *expecting != ExpectLabel && *expecting != ExpectDigitOrEnd) {
				if (*expecting == ExpectDollarSign)
					endStatus = IllegalSymbolFlag; /* A label symbol cannot start with a digit. */
				else
					endStatus = StrayDigitFlag; /* Just in case. */
				break; /* Found illegally positioned digit. */
			}
			if (*isRegister)
				*expecting = ExpectDigitOrEnd;
			isSpaceAllowed = 1; /* Spaces would indicate that the operand is over. */
			continue;
		}
		if (isalpha(c)) {
			if (*expecting != ExpectLabel && *expecting != ExpectDollarSign) { /* A label can appear instead of a register. */
				endStatus = UnexpectedFlag;
				break; /* Found an unexpected token; */
			}
			*expecting = ExpectLabel; /* The operand is a label. */
			isLineEmpty = 0; /* The line is not empty. */
			continue;
		}
		endStatus = UnexpectedFlag;
		break; /* Found an unexpected token, that is any other character. */
	}

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || (*expecting != ExpectDigitOrEnd && *expecting != ExpectLabel))
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	return endStatus;
}

/**
 * Uses the given range to scan for the operand from the given line and
 * returns it through one of the last two parameters.
 * If the operand is a label then reg would remain untouched,
 * and if the operand is a register then the last parameter
 * would be a null pointer.
 * This function may or may not allocate memory on the heap for the last
 * parameter.
 * This function also check if the register address is valid, if it is
 * not then InvalidRegisterFlag flag would be returned and no memory
 * would be allocated, same for the length of the label.
 * Expects the second parameter to be set to the index where the J
 * operand definition begins and the fifth parameter to be set to 1
 * if the operand to extract is a register and 0 if it is a label.
 */
Flag extractJParam(char *sourceLine, const int startIndex, const int endIndex, const char isRegister, char *reg, char **label) {
	int destIndex; /* To track the temporary array of the symbol */
	int symbolSize; /* To store the length of the symbol array. */
	long int value = 0; /* To store every extracted value to check limits before assignment. */
	char *pointer; /* A pointer to track the position on the line. */
	char *temp = NULL; /* A temporary pointer to point to the symbol of the label if there is one. */
	Flag endStatus = NoIssueFlag; /* To detect issues in the source line. */

	if (isRegister) { /* The Operand is a register. */
		/* Setting the pointer to the first digit of the operand right after the dollar sign. */
		pointer = sourceLine + startIndex + 1;

		value = strtol(pointer, &pointer, DECIMAL); /* Extracting the operand. */
		/* Checking if the operand is valid. */
		if (value < 0 || value > MAX_REGISTER)
			return InvalidRegisterFlag; /* The operand is not valid. */
		*reg = value; /* The operand is valid. */
	} else { /* The Operand is a label. */
		pointer = sourceLine + startIndex;
		symbolSize = endIndex - startIndex + 2;
		if (symbolSize > MAX_REGISTER + 1) /* + 1 for a terminating character. */
			return IllegalSymbolFlag; /* Label symbols cannot be longer than 31. */
		if ((temp = malloc(symbolSize)) == NULL) /* Allocating memory to store the symbol. */
			return HardwareErrorFlag; /* Cannot continue without memory. */

		/* The following loop copies the symbol from the given line to the temporary array. */
		for (destIndex = 0; destIndex < symbolSize - 1; destIndex++, pointer++){
			temp[destIndex] = *pointer;
		}
		/* Adding the terminating character at the end. */
		temp[destIndex] = TERMINATING_CHAR;
	}

	*label = temp; /* Assigning the symbol to the return parameter, if there was one. Null if otherwise. */
	return endStatus;
}

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
Flag getJParam(char *sourceLine, Expectation *expecting, int *index, char *reg, char **label) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	char isRegister = 0;
	Flag endStatus; /* To detect issues in the source line. */

	/* Getting the range of indexes between which the operand is defined. */
	endStatus = rangeJParam(sourceLine, expecting, &isRegister, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the character after the operand. */

	/* If no syntax issues were found the operand would be extracted. */
	if (endStatus == NoIssueFlag && (*expecting == ExpectDigitOrEnd || *expecting == ExpectLabel))
		endStatus = extractJParam(sourceLine, startIndex, endIndex, isRegister, reg, label);
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



/**
 * Uses the given start index to scan for the word from the given line
 * and returns it  through the last parameter, while checking for
 * sizes issues.
 * if there are no issues the return flag should be NoIssueFlag flag.
 * Expects the second parameter to be expectEnd.
 * This information is used for the word size checking.
 * The third parameter should be pointing to the position before the
 * word start.
 * This function allocates memory on the heap for the last parameter.
 */
Flag extractWord(char *sourceLine, const Expectation expecting, const int startIndex, const int endIndex, void **args) {
	Flag endStatus = NoIssueFlag;
	int index; /* Index on the line (source). */
	int paramIndex = 0; /* Index on the word parameter (destination). */
	char *temp; /* Temporary array to copy the word to. */

	temp = malloc(endIndex - startIndex); /* Allocating memory for the word. */
	if (temp == NULL)
		endStatus = HardwareErrorFlag; /* Cannot continue without memory. */

	/* The following loop copies the word from the given line to the temporary array. */
	for (index = startIndex + 1; index < endIndex; index++, paramIndex++){
		temp[paramIndex] = sourceLine[index];
	}

	/* Adding the terminating character at the end. */
	temp[paramIndex] = TERMINATING_CHAR;
	*args = temp; /* Setting the last parameter to point to that array. */
	return endStatus;
}


/**
 * Scans a portion from the given source line to find the range of indexes
 * between which the word is located, all while checking for syntax errors.
 * Should return NoIssueFlag flag if there was no issue. the second
 * parameter should be set to ExpectEnd, and the two last parameters set
 * to the range of indexes between which the word is coded (including).
 * In case of a syntax error, the returned flag and the second parameter
 * can be used to determine what's the problem  while the last two parameters would
 * be equal to the index where the issue was found.
 * The function expects the third parameter to be set to the index which the scan
 * should begin from.
 */
Flag rangeWord(char *sourceLine, Expectation *expecting, int *startIndex, int *endIndex) {
	int index; /* Tracks the index of every character in the given line. */
	char c; /* For readability purposes. */
	char isLineEmpty = 0; /* check if The line is empty. */

	Flag endStatus = NoIssueFlag; /* To detect issues in the source code. */
	*expecting = ExpectWord; /* Expecting a vaild Label, instructor or operator. */

	for (index = *startIndex; sourceLine[index] != (TERMINATING_CHAR || NEW_LINE); index++) {
		c = sourceLine[index];

		if (c == SPACE && isLineEmpty == 0) { /* Current character is space, but no text had readen yet. */
			/*
			 * Setting the beginning of the range based on if the current position
			 * is before the parameters definition.
			 */
			*startIndex = index + 1;
			continue;
		}
		if (c == COMMENT) { /* Current character is a semicolon. */
			*expecting = ExpectComment;
			break; /* A comment must have a dedicated line. */
		}
	
		if (isalpha(c)) { /* Current character is a letter. */
			isLineEmpty = 1;  /* The line is not empty. */
			*expecting = ExpectDigitOrletter; /* The next character can be a digit or a letter. */
			continue;
		}
		if (c == SPACE && isLineEmpty == 1) { /* Current character is space , so it's the end of word. */
			break;
		}
		
		endStatus = UnexpectedFlag;
		break; /* Found an unexpected character. */
	}
	

	*endIndex = index - 1; /* Setting the end of the range. */

	/* Checking if an issue was found. */
	if (endStatus != NoIssueFlag || (*expecting != ExpectDigitOrletter && *expecting != ExpectComment))
		*startIndex = *endIndex = index; /* Getting the position of the issue. */
	else
		*expecting = ExpectEnd; /* Making checking for issues easier outside this function. */

	return endStatus;
}

/**
 * Scans a portion of the given source line and extracts the word
 * into the last parameter if there were no syntax errors.
 * In case of a syntax error, the function can be extracted using the returned flag
 * and the third parameter would point to the index where the issue was found.
 * In case there were no issues, the last parameter would contain the extracted
 * word, the third parameter would point to the index with the terminating character,
 * (the end of the line).
 * Expects the third parameter to be positioned before the word and the
 * second parameter to be ExpectEnd. This information will be useful for the map 
 * and convert functions.
 * 
 */
Flag getWord(char *sourceLine, Expectation *expecting, int *index, void **args) {
	int startIndex = *index; /* The beginning of the range. */
	int endIndex = *index; /* The ending of the range. */
	Flag endStatus; /* To detect issues in the source line. */
	Expectation sizeExpectation = *expecting; /* to know the size of the word */

	/* Getting the range of indexes between which the word are exist. */
	endStatus = rangeWord(sourceLine, expecting, &startIndex, &endIndex);
	*index = endIndex + 1; /* Setting the index to the position of the terminating character. */

	/* If no syntax issues were found the word would be extracted. */
	if (endStatus == NoIssueFlag && *expecting == ExpectEnd)
		endStatus = extractWord(sourceLine, sizeExpectation, startIndex, endIndex, args);
	else
		/* If there is a syntax error then the index would be set to its position. */
		*index = endIndex;
	return endStatus;
}
