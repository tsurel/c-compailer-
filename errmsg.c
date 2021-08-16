#include <stdio.h>
#include <stdlib.h>

#include "errmsg.h"
#include "asmutils.h"

/**
 * The errmsg translation unit is responsible for printing error
 * messages for the user to see.
 */

/**
 * Prints a title for the error message with the given file name
 * and the given line number and the given index.
 * The title does not include a new line character.
 */
void printMsgTitle(const char *fileName, unsigned long int line, int index) {
	printf("%s:%ld:%d: ", fileName, line, index);
}

/**
 * Prints the given source line with an pointer underneath the
 * position of the issue (using the given index) and, the line
 * number at the beginning.
 */
void printLine(const char *sourceLine, unsigned long int line, int index) {
	int i; /* For the loop that places the pointer. */
	printf("%08ld |%s\n", line, sourceLine); /* Printing the line. */

	/* Printing the pointer bellow the position of the issue. */
	for (i = 1; i < index && i < SOURCE_LINE_LENGTH; i++)
		printf(" ");
	printf("^\n");
}

/**
 * Checks for issues that may be found in the source file
 * after calling the extractSourceLine function.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckLine(const char *fileName, const char *sourceLine, unsigned long int line, int length, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	printMsgTitle(fileName, line, SOURCE_LINE_LENGTH); /* Printing error message title. */

	/* Figuring the error message. */
	if (status == WarningLineLengthFlag) {
		printf("Warning: character limit exceeded on this line by %d blank characters\n", length - SOURCE_LINE_LENGTH);
		event = WEvent; /* The event is a warning. */
	} else if (status == ErrorLineLengthFlag) {
		printf("Error: character limit exceeded on this line by %d characters\n", length - SOURCE_LINE_LENGTH);
		event = EEvent; /* The event is an error. */
	}

	printLine(sourceLine, line, length); /* Printing the line. */

	return event;
}

/**
 * For extreme cases, should never be called.
 * For cases where there is an hardware related issue, such as
 * a memory allocation failure, that prevents the program from
 * functioning properly.
 */
void errFatal() {
	fprintf(stderr, "FATAL: Internal failure, the program will now exit.\n");
	exit(EXIT_FAILURE); /* Cannot continue the program at this point. */
}

/**
 * Prints an unexpected token error message.
 */
void errUnexpected() {
	printf("Error: unexpected token, delete this token\n");
}

/**
 * Checks for issues that may be found in the source file
 * after calling the getDataParam function.
 * Uses the last two parameters for the error checking
 * and the rest of the parameters for the error message.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckData(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if (status == NoIssueFlag && expecting == ExpectEnd)
		return event; /* No errors were found, nothing was printed. */

	if (status == HardwareErrorFlag) /* An hardware related issue had occurred. */
		errFatal();

	event = EEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == NoIssueFlag) { /* The line is incomplete. */
		if (expecting == ExpectDigitOrSign) /* The line ended empty or after a comma. */
			printf("Error: argument is expected here\n");
		else if (expecting == ExpectDigit) /* The line ended after a plus or a minus. */
			printf("Error: incomplete argument\n");
	} else if (status == IllegalSpacingFlag) { /* The line has illegal spacing. */
		printf("Error: illegal spacing\n");
	} else if (status == StrayCommentFlag) { /* There is a semicolon among the arguments. */
		printf("Error: a comment must have a dedicated line\n");
	} else if (status == SizeOverflowFlag) { /* One (or more) of the arguments was too large for the specified data instruction. */
		printf("Warning: argument size is too large, only least significant bytes were scanned\n");
		event = WEvent; /* The event is a warning. */
	} else { /* StraySignFlag, UnexpectedFlag, StrayDigitFlag. The line has unexpected tokens that should be deleted. */
		errUnexpected();
	}

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors or warnings were found. */
}

/**
 * Checks for issues that may be found in the source file
 * after calling the getAscizParam function.
 * Uses the last two parameters for the error checking
 * and the rest of the parameters for the error message.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckAsciz(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if (status == NoIssueFlag && expecting == ExpectEnd)
		return event; /* No errors were found, nothing was printed. */

	if (status == HardwareErrorFlag) /* An hardware related issue had occurred. */
		errFatal();

	event = NEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == NoIssueFlag) { /* ExpectQuote, there is no string, the line is empty. */
		printf("Error: string is expected\n");
	} else if (status == IncompleteStringFlag) {
		printf("Error: string definition lacks ending quote\n");
	} else { /* UnexpectedFlag, non-quote characters have appeared before the string. */
		errUnexpected();
	}

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors or warnings were found. */
}

/**
 * Checks for issues that may be found in the source file
 * after calling the getRParam function.
 * Uses the last two parameters for the error checking
 * and the rest of the parameters for the error message.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckR(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if (status == NoIssueFlag && expecting == ExpectDigitOrEnd)
		return event; /* No errors were found, nothing was printed. */

	event = EEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == NoIssueFlag) {
		if (expecting == ExpectDollarSign) { /* The line ended before all operands were declared. */
			printf("Error: operand is expected\n");
		} else if (expecting == ExpectDigitOrComma || expecting == ExpectComma) { /* The line ended before all operands were declared. */
			printf("Error: missing operands, operand separation ',' is expected\n");
		} else { /* ExpectDigit, nothing after a dollar sign. */
			printf("Error: incomplete operand\n");
		}
	} else if (status == IllegalSpacingFlag) { /* Spaces are not allowed between a dollar sign and register address. */
		printf("Error: illegal spacing, spacing is not allowed after this token\n");
	} else if (status == StrayCommentFlag) { /* Found a semicolon on a code line. */
		printf("Error: a comment must have a dedicated line\n");
	} else if (status == InvalidRegisterFlag) { /* Found a defined register with invalid address. */
		printf("Error: invalid register address, valid addresses are 0 - 31\n");
	} else { /* StrayDollarSignFlag, StrayDigitFlag, StrayCommaFlag, UnexpectedFlag flags. Tokens should not be there. */
		errUnexpected();
	}

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors were found. */
}

/**
 * Checks for issues that may be found in the source file
 * after calling the getIParam function.
 * Uses the last two parameters for the error checking
 * and the rest of the parameters for the error message.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckI(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if (status == NoIssueFlag && (expecting == ExpectDigitOrEnd || expecting == ExpectLabel))
		return event; /* No errors were found, nothing was printed. */

	if (status == HardwareErrorFlag) /* An hardware related issue had occurred. */
		errFatal();

	event = EEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == NoIssueFlag) {
		if (expecting == ExpectDollarSign) { /* The line ended with missing operand(s). */
			printf("Error: operand is expected\n");
		} else if (expecting == ExpectDigitOrComma || expecting == ExpectComma) { /* The line ended before all operands were declared. */
			printf("Error: missing operands, operand separation ',' is expected\n");
		} else { /* ExpectDigit, ExpectDigitOrSignOrDollarSign, The line ended with an incomplete operand. */
			printf("Error: incomplete operand, digit is expected\n");
		} 
	} else if (status == IllegalSpacingFlag) { /* Spaces are not allowed between a dollar sign and register address as well as between signs and digits. */
		printf("Error: illegal spacing, spacing is not allowed after this token\n");
	} else if (status == StrayCommentFlag) { /* Found a semicolon on a code line. */
		printf("Error: a comment must have a dedicated line\n");
	} else if (status == SizeOverflowFlag) { /* The immediate value was too large. */
		printf("Warning: argument size is too large, only least significant bytes were scanned\n");
		event = WEvent; /* This is a warning event. */
	} else if (status == IllegalSymbolFlag) { /* The specified symbol is illegal. */
		printf("Error: The specified symbol is illegal\n");
	} else if (status == InvalidRegisterFlag) { /* Found a defined register with invalid address. */
		printf("Error: invalid register address, valid addresses are 0 - 31\n");
	} else { /* StrayDollarSignFlag, StraySignFlag, StrayDigitFlag, StrayCommaFlag, UnexpectedFlag flags. Tokens should not be there. */
		errUnexpected();
	}

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors were found. */
}

/**
 * Checks for issues that may be found in the source file
 * after calling the getJParam function.
 * Uses the last two parameters for the error checking
 * and the rest of the parameters for the error message.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckJ(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if (status == NoIssueFlag && (expecting == ExpectDigitOrEnd || expecting == ExpectLabel))
		return event; /* No errors were found, nothing was printed. */

	if (status == HardwareErrorFlag) /* An hardware related issue had occurred. */
		errFatal();

	event = EEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == NoIssueFlag) {
		if (expecting == ExpectDollarSign) { /* The line ended with missing operand (the operand could also be a label). */
			printf("Error: operand is expected\n");
		} else { /* ExpectDigit, The line ended with incomplete operand. */
			printf("Error: incomplete operand\n");
		}
	} else if (status == IllegalSpacingFlag) { /* Spaces are not allowed between a dollar sign and register address. */
		printf("Error: illegal spacing, spacing is not allowed after this token\n");
	} else if (status == StrayCommentFlag) { /* Found a semicolon on a code line. */
		printf("Error: a comment must have a dedicated line\n");
	} else if (status == IllegalSymbolFlag) { /* The specified symbol is illegal. */
		printf("Error: The specified symbol is illegal\n");
	} else if (status == InvalidRegisterFlag) { /* Found a defined register with invalid address. */
		printf("Error: invalid register address, valid addresses are 0 - 31\n");
	} else { /* StrayDollarSignFlag, StrayDigitFlag, UnexpectedFlag, Tokens should not be there. */
		errUnexpected();
	}

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors were found. */
}

/**
 * Unfinished
 */
Event errCheckWord(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status) {
	Event event = NEvent; /* No errors by default, used for tracking the error type. */
	if ((status == LabelFlag || status == InstructorFlag || status == OperatorFlag || status == CommentLineFlag) && expecting == ExpectEnd)
		return event; /* No errors were found, nothing was printed. */

	if (status == HardwareErrorFlag) /* An hardware related issue had occurred. */
		errFatal();

	event = EEvent; /* The event is an error. */
	printMsgTitle(fileName, line, index); /* Printing error message title. */
	/* Figuring the error message. */
	if (status == IllegalSpacingFlag) { /* Illegal spacing can be returned only it there was a space after a dot. */
		printf("Error: Illegal spacing, data instructor is expected\n");
	} else if (status == StrayCommentFlag || status == UnexpectedFlag || status == StrayDigitFlag) { /* Found a semicolon on a code line. */
		errUnexpected();
	} /* TODO: Add more checks. */

	printLine(sourceLine, line, index); /* Printing the line. */

	return event; /* Errors were found. */
}
