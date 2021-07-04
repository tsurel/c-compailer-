#include <string.h>

#include "errorhandler.h"
#include "utils.h"

#define FILE_EXTENSION ".as"
#define FILE_EXTENSION_LEN 3

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
