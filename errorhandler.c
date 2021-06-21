#include <string.h>

#include "errorhandler.h"
#include "utils.h"

#define FILE_EXTENSION ".as"
#define FILE_EXTENSION_LEN 3

Code isValid(const char *fileName) {
	int nameLength = strlen(fileName);
	char extension[FILE_EXTENSION_LEN + 1];

	subString(extension, fileName, nameLength - FILE_EXTENSION_LEN, nameLength);

	return strcmp(extension, FILE_EXTENSION) == 0 ? SUCCESS : ERROR;

}
