#include <stdio.h>

#include "converter.h"
#include "errorhandler.h"

/**
Assembles the content of the source files, provided as arguments, from assembly code into machine code.
*/

/**
 * The assembler starts here with the file names provided as command line
 * arguments. The main method is responsible for opening the assembly source files
 * as streams and passing them forward to where they will be assembled. At the end
 * it closes every stream.
 */
int main(int argc, char const *argv[]) {

	int i;
	FILE *file;

	/* Relevant arguments starts at 1. */
	for (i = 1; i < argc; i++) {

		/* Checking if the file extension is valid. */
		if (isValid(argv[i]) == ERROR) {
			/* Skipping the file if it is not an assembly source code file. */
			printf("%s%s\n", "Invalid file extension: ", argv[i]);
			continue;
		}

		/* Opening the file to assemble as a stream. */
		file = fopen(argv[i], "r");
		if (file == NULL) {
			/* Skipping the file if it is not accessible. */
			printf("%s%s\n", "Could not access this file: ", argv[i]);
			continue;
		}
		/* Assembling the file. */
		assemble(file);
		/* Closing the file. */
		fclose(file);
	}

	return 0;
}
