#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"
#include "utils.h"
#include "symboltable.h"
#include "asmutils.h"
#include "keywords.h"

/**
 * The converter translation unit is responsible for managing the assembling
 * process.
 */

#define OUTPUT_OB_EXTENTION ".ob" /* Output object file extension for assembled source files. */
#define OUTPUT_ENT_EXTENTION ".ent" /* Output entries file extension for assembled source files. */
#define OUTPUT_EXT_EXTENTION ".ext" /* Output externals file extension for assembled source files. */

/**
 * The following functions should not be used outside this translation unit.
 */
Code map(FILE *file, SymbolTable *symboltable);
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, const unsigned long int IC, const unsigned long int DC);
void extractOutputFileNames(const char *sourceFileName, char **obFileName, char **entFileName, char **extFileName);
char *assembleR(Operator *op, char rs, char rt, char rd);
char *assembleI(Operator *op, char rs, char rt, short immed);
char *assembleJ(Operator *op, unsigned char reg, unsigned address);
/* TODO: Implement the assembler itself using the functions above. */

/* __Temporary test code__ */
void test(FILE *file) {
	Flag flag;
	Expectation expecting = Expect8BitParams;
	int index = -1;
	char *line = malloc(SOURCE_LINE_LENGTH + 1);
	char *str = NULL;

	if (line == NULL)
		exit(EXIT_FAILURE);

	flag = extractSourceLine(file, line, &index);
	printf("%s\n", line);

	index = 0;
	flag = getWord(line, &expecting, &index, &str);

	printf("%d\t%d\t%d\n", flag, expecting, index);
	if ((flag == LabelFlag || flag == InstructorFlag || flag == OperatorFlag) && expecting == ExpectEnd) {
		if (str != NULL) {
			printf("%s\n", str);
			free(str);
		}
	}

	index = -1;
	flag = extractSourceLine(file, line, &index);
	printf("%s\n", line);
	index = 0;
	flag = getAscizParam(line, &expecting, &index, &str);
	printf("%d\t%d\t%d\n", flag, expecting, index);
	if (str != NULL)
		printf("%s\n", str);

	free(line);
}

void assemble(FILE *file, const char *fileName) {
	test(file);
	convert(file, fileName, NULL, 0, 0);
}

Code map(FILE *file, SymbolTable *symboltable) {
	return 0;
}

/* Unfinished and untested, requires map to be tested. */
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, const unsigned long int IC, const unsigned long int DC) {
	/*int index;
	int lengthCheck;
	char shouldStop = 0;
	char *word = NULL;
	char rs, rt, rd;
	char *dataSegment;
	FILE *outputObj;
	FILE *outputEnt = NULL, *outputExt = NULL;*/
	char *obFileName, *entFileName, *extFileName;
	/*Operator *operator;
	Instructor *Instructor;
	Expectation expecting;
	Flag status;*/

	/* Getting the names of the output files. */
	extractOutputFileNames(fileName, &obFileName, &entFileName, &extFileName);

	/* TODO: Add the conversion loop here. */

	/* Freeing memory. */
	free(obFileName);
	free(entFileName);
	free(extFileName);
}

/**
 * Extracts the file name without the extension from the first parameter and
 * assigns that name as a string into the last three parameters, as well as
 * assigning them their own extensions: .ob .ent .ext respectively.
 * This function allocates memory on the heap for the last three buffers.
 * This is a private utility function for the convert function.
 */
void extractOutputFileNames(const char *sourceFileName, char **obFileName, char **entFileName, char **extFileName) {
	const char nullTermination = '\0'; /* Null terminating character, used for string handling. */
	char *srcFileName; /* To contain the source file name without the extension. */
	int sourceFileNameLen = strlen(sourceFileName); /* Used in multiple locations. */

	/* Allocating memory for the strings that should store the output file names. */
	if ((*obFileName = malloc(sourceFileNameLen - FILE_EXTENSION_LEN + strlen(OUTPUT_OB_EXTENTION) + 1)) == NULL) {
		exit(EXIT_FAILURE); /* Cannot continue without memory. */
	}
	if ((*entFileName = malloc(sourceFileNameLen - FILE_EXTENSION_LEN + strlen(OUTPUT_ENT_EXTENTION) + 1)) == NULL) {
		exit(EXIT_FAILURE); /* Cannot continue without memory. */
	}
	if ((*extFileName = malloc(sourceFileNameLen - FILE_EXTENSION_LEN + strlen(OUTPUT_EXT_EXTENTION) + 1)) == NULL) {
		exit(EXIT_FAILURE); /* Cannot continue without memory. */
	}
	if ((srcFileName = malloc(sourceFileNameLen - FILE_EXTENSION_LEN)) == NULL) {
		exit(EXIT_FAILURE); /* Cannot continue without memory. */
	}

	/* Extracting the source file's name without the extension into srcFileName. */
	subString(srcFileName, sourceFileName, 0, sourceFileNameLen - FILE_EXTENSION_LEN - 1);

	/* Assigning a terminating character at the beginning of each buffer before using strncat. */
	(*obFileName)[0] = nullTermination;
	(*entFileName)[0] = nullTermination;
	(*extFileName)[0] = nullTermination;

	/* Copying the name of the file without the extension into the buffers. */
	strcat(*obFileName, srcFileName);
	strcat(*entFileName, srcFileName);
	strcat(*extFileName, srcFileName);

	/* Assigning the right extension for every output file name. */
	strcat(*obFileName, OUTPUT_OB_EXTENTION);
	strcat(*entFileName, OUTPUT_ENT_EXTENTION);
	strcat(*extFileName, OUTPUT_EXT_EXTENTION);

	/* Freeing memory */
	free(srcFileName);
}
