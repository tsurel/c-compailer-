#include <stdio.h>
#include <string.h>

#include "asmutils.h"
#include "utils.h"

/*
 * Contains a collection of utility functions for this assembler.
 * These functions are relevant for reading from the source file.
 */

#define FILE_EXTENSION ".as" /* The extension for the assembly source files. */
#define FILE_EXTENSION_LEN 3 /* The length of the assembly source file extension. */

/**
 * Scans from an assembly source file for an isolated sequence of characters
 * that is between white characters and returns a source word data structure
 * of it. This function can also find syntax errors.
 */
/* NOT FINISHED, may add another parameter (name: expected). */
Flag nextSourceWord(FILE *sourceFile, SourceWord *sourceWord) {
	char c; /* To track every character from the given stream. */
	unsigned char isEmptyOrComment = 1; /* To track if the current line has code. */
	unsigned char isLabel = 0; /* To track if the source word is a label. */
	long wordStart; /* The position on the stream where the source word starts. */
	unsigned length = 0; /* The length of the word. */
	Flag returnFlag;

	while ((c = fgetc(sourceFile)) != EOF) {
		if (isEmptyOrComment) {
			if (c == ';' || c == '\n')
				break;
			if (c == ' ' || c == '\t')
				continue;
			isEmptyOrComment = 0;
			wordStart = ftell(sourceFile) - 1;
		} else {
			if (c == ';' || c == '\n')
				break;
			if (c == ':') {
				isLabel = 1;
				continue;
			}
			if (c == ' ' || c == '\t')
				break;
			if (isLabel)
				returnFlag = NO_SPACE_AFTER_LABEL;
		}
	}
	
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
