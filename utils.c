#include <stdio.h>

#include "utils.h"

/**
 * Contains a collection of general utility method that can be
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
