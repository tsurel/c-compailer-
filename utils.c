#include <stdio.h>

#include "utils.h"

/**
 * Contains a collection of general utility functions that can be
 * used anywhere in the program.
 */

/**
 * Skips the current line of the given file stream without storing it.
 */
void skipln(FILE *stream) {
	char c;
	/* Reading every digit from the provided stream until hitting a new line or an end of file character. */
	for (c = fgetc(stream); c != '\n' && c != EOF; c = fgetc(stream));
}

/**
 * Copies a sub-string from the source parameter into the destination parameter,
 * starting from the given start index until the given end index (included).
 */
void subString(char *dest, const char *src, int start, int end) {
	int i = 0; /* The index for the destination*/

	while (start <= end) {
		/* Copying every character while incrementing the indexes. */
		dest[i++] = src[start++];
	}
}
