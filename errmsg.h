#ifndef ERRMSG_H
#define ERRMSG_H

#include "asmutils.h"

/**
 * An header file for the errmsg translation unit.
 */

/**
 * A return type for functions in the errmsg translation unit.
 * Used for handling assembly coding errors that may prevent
 * outputting assembled output files. This enumeration is used
 * for distinguishing different cases.
 */
typedef enum {
	EEvent, /* An error message was printed, no output should be created. */
	WEvent, /* A warning message was printed, does not affect the output. */
	NEvent /* No message was printed, does not affect the output. */
} Event;

/**
 * Checks for issues that may be found in the source file
 * after calling the extractSourceLine function.
 * If an issue was found an error message would be
 * printed.
 * Returns an Event enumeration value that can be used
 * to determine what was the issue (if there was one).
 */
Event errCheckLine(const char *fileName, const char *sourceLine, unsigned long int line, int length, Flag status);

/**
 * For extreme cases, should never be called.
 * For cases where there is an hardware related issue, such as
 * a memory allocation failure, that prevents the program from
 * functioning properly.
 */
void errFatal();

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
Event errCheckData(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

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
Event errCheckAsciz(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

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
Event errCheckR(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

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
Event errCheckI(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

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
Event errCheckJ(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

/**
 * Unfinished
 */
Event errCheckWord(const char *fileName, const char *sourceLine, unsigned long int line, int index, Expectation expecting, Flag status);

#endif
