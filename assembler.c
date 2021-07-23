#include <stdio.h>
#include <stdlib.h>

#include "converter.h"
#include "asmutils.h"
#include "keywords.h"

/**
 * Assembles the content of the source files, provided as arguments, from assembly
 * code into machine code.
 */

/**
 * The assembler starts here with the file names provided as command line
 * arguments. The main function is responsible for opening the assembly source files
 * as streams and passing them forward to where they will be assembled. At the end
 * it closes every stream.
 */
int main(int argc, char const *argv[]) {

	int index;
	FILE *file; /* Used for accessing files as streams. */

	/* Initializing the assembly keywords container. */
	if (initasmKeywords() == ERROR) {
		/* If the initialization failed the assembler cannot run. */
		printf("Initialization error: essential memory allocation has failed.");
		exit(EXIT_FAILURE);
	}

	/* Relevant arguments starts at 1. */
	for (index = 1; index < argc; index++) {

		/* Checking if the file extension is valid. */
		if (isValid(argv[index]) == ERROR) {
			/* Skipping the file if it is not an assembly source code file. */
			printf("%s%s\n", "Invalid file type: ", argv[index]);
			continue;
		}

		/* Opening the file to assemble as a stream. */
		file = fopen(argv[index], "r");
		if (file == NULL) {
			/* Skipping the file if it is not accessible. */
			printf("%s%s\n", "Could not access this file: ", argv[index]);
			continue;
		}
		/* Assembling the file. */
		assemble(file);
		/* Closing the file. */
		fclose(file);
	}

	/* Freeing all the memory used by the assembly keywords container. */
	clearasmKeywords();

	return 0;
}
