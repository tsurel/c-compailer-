#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"
#include "utils.h"
#include "symboltable.h"
#include "asmutils.h"
#include "keywords.h"
#include "errmsg.h"

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
Code map(FILE *file, SymbolTable *symboltable, char *sourceLine);
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, char *sourceLine, const unsigned long int ic, const unsigned long int dc);
void extractOutputFileNames(const char *sourceFileName, char **obFileName, char **entFileName, char **extFileName);
void assembleR(FILE *output, unsigned long int address, Operator *op, char rs, char rt, char rd);
void assembleI(FILE *output, unsigned long int address, Operator *op, char rs, char rt, short immed);
void assembleJ(FILE *output, unsigned long int address, Operator *op, char isRegister, unsigned long int addressValue);
void assembleAsciz(char *dataSegment, char *str, unsigned long int *startIndex);
void assembleData(char *dataSegment, unsigned long int *startIndex, const Expectation expecting, const int count, void *args);
void writePlain(FILE *output, char *symbol, unsigned long int address);
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
	printf("%s\n%d\n", line, flag);
	index = 0;
	flag = getAscizParam(line, &expecting, &index, &str);
	printf("%d\t%d\t%d\n", flag, expecting, index);
	if (str != NULL)
		printf("%s\n", str);

	free(line);
}

void assemble(FILE *file, const char *fileName) {
	test(file);
}

/* Unfinished. */
Code map(FILE *file, SymbolTable *symboltable, char *sourceLine) {
	Code code = SUCCESS; /* Error code to track issues. */
	char shouldStop = 0; /* To track when the loop should stop meaning, the source file has ended */
	int index; /* An index to track the position on the line. */
	unsigned long int lineNum = 0; /* To track the line number. */
	/*Expectation expecting;  To differentiate different situations and catch issues. */
	Flag status; /* To differentiate different situations and catch issues. */

	while (!shouldStop) {
		index = -1; /* The line start at index 0. */
		lineNum++; /* This is a new line. */

		if ((status = extractSourceLine(file, sourceLine, &index)) == EndFileFlag) /* TODO, adjust extractSourceLine function, @PARAM Event. */
			shouldStop = 1; /* This is the last line in the source file. */

		index = 0;
	}

	return code;
}

/* Unfinished and untested, requires map to be tested. */
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, char *sourceLine, const unsigned long int ic, const unsigned long int dc) {
	const char assembledLineSize = 4; /* The size for the bit field in the output file. */
	const char *stopOperator = "stop"; /* Special case keyword, no operands. */
	int index; /* An index to track the position on the line. */
	int lengthCheck = -1; /* A variable to use the extractSourceLine function. */
	int count; /* Used for counting arguments for db, dh, and dw keywords. */
	unsigned long int address = 100; /* To track the memory address of the assembled operators in the output file. */
	char shouldStop = 0; /* To track when the loop should stop meaning, the source file has ended */
	char *word = NULL; /* A variable to store the labels\Instructors\Operators returned from getWord. */
	char *symbol = NULL; /* A variable to store the label operand of I\J operators. May be used for string holding as well. */
	char rs, rt, rd; /* Variables to store register addresses. */
	short immed; /* A variable to store the immediate value for I operators. */
	char *dataSegment; /* Points to the array that stores all the assembled data instructors parameters. */
	void *args = NULL; /* To store and access db\dh\dw arguments. */
	unsigned long int dataSegmentIndex = 0; /* Index variable for the data segment array. */
	FILE *outputObj;
	FILE *outputEnt = NULL, *outputExt = NULL;
	char *obFileName, *entFileName, *extFileName;
	SymbolTable *label; /* A variable for label handling. */
	Operator *operator; /* To hold operators. */
	Instructor *instructor; /* To hold instructors. */
	Expectation expecting; /* To use functions and track data instruction expectation. */
	Expectation sizeExpectation; /* Used for extracting data arguments. */
	Flag status; /* To differentiate different situations and catch memory allocation issues. */

	/* Getting the names of the output files. */
	extractOutputFileNames(fileName, &obFileName, &entFileName, &extFileName);

	outputObj = fopen(obFileName, "w+"); /* Creating/recreating the output file. */
	if (outputObj == NULL)
		errFatal(); /* cannot continue without the output file. */
	if ((dataSegment = malloc(dc)) == NULL) { /* Allocating memory for the data segment. */
		errFatal(); /* Cannot continue without memory. */
	}

	fprintf(outputObj, "     %lX %lX\n", ic, dc);

	while (!shouldStop) {
		index = 0; /* The line start at index 0. */
		rs = rt = rd = 0; /* Avoid problems in functions that use registers. */

		if ((status = extractSourceLine(file, sourceLine, &lengthCheck)) == EndFileFlag)
			shouldStop = 1; /* This is the last line in the source file. */

		if ((status = getWord(sourceLine, &expecting, &index, &word)) == LabelFlag) { /* Extracting the beginning of the line. */
			free(word); /* Avoiding memory leak. */
			word = NULL; /* Just in case. */
			status = getWord(sourceLine, &expecting, &index, &word); /* Extracting again if it was a label. */
		} else if (status == CommentLineFlag)
			continue; /* Skipping comment line. */
		else if (status == NoIssueFlag && expecting == ExpectWord)
			continue; /* The line is empty. */

		if (status == OperatorFlag) {
			operator = searchOperatorByString(word); /* Getting the operator. */
			if (getType(operator) == R) { /* Handling R type operators. */
				if (getOpcode(operator))
					/* Extracting 2 operands. */
					getRParam(sourceLine, &expecting, R2, &index, &rs, &rt, &rd);
				else
					/* Extracting 3 operands. */
					getRParam(sourceLine, &expecting, R3, &index, &rs, &rt, &rd);
				assembleR(outputObj, address, operator, rs, rt, rd); /* Assembling the line. */
			} else if (getType(operator) == I) { /* Handling I type operators. */
				/* Extracting the data from the line as operand set for I operators. */
				status = getIParam(sourceLine, &expecting, &index, &rs, &rt, &immed, &symbol);
				if (status == HardwareErrorFlag) {
					errFatal(); /* Memory allocation had failed, cannot continue the program. */
				}
				if (symbol != NULL) { /* If one of the operands is a label. */
					label = searchLabel(symboltable, symbol); /* Extracting the label. */
					immed = getAddress(label) - address; /* Calculating the difference into the immediate field. */
					free(symbol); /* Freeing the symbol. */
					symbol = NULL; /* Avoiding problems for this if statement. */
				} /* If there was no label no special treatment is required. */
				assembleI(outputObj, address, operator, rs, rt, immed); /* Assembling the line. */
			} else if (strcmp(word, stopOperator) == 0) { /* Special case, the "stop" keyword. */
				assembleJ(outputObj, address, operator, 0, 0); /* The "stop" keyword takes no operands. */
			} else { /* The remaining operators must be of type J. */
				/* Extracting the data from the line. */
				status = getJParam(sourceLine, &expecting, &index, &rs, &symbol);
				if (status == HardwareErrorFlag) {
					errFatal(); /* Memory allocation had failed, cannot continue the program. */
				}
				if (symbol != NULL) { /* If the operand is a label. */
					label = searchLabel(symboltable, symbol); /* Extracting the label from the symbol table. */
					free(symbol); /* The symbol is no longer necessary. */
					symbol = NULL; /* Avoiding problems for this if statement. */
					if (hasAttribute(label, EntryLabel) == SUCCESS) { /* This may be an entry label. */
						if (outputEnt == NULL) { /* If that file was not created yet then it would be created. */
							outputEnt = fopen(entFileName, "w+"); /* Creating\recreating the output file. */
							if (outputEnt == NULL)
								errFatal(); /* should not happen but, just in case. */
						}
						writePlain(outputEnt, getSymbol(label), address); /* Writing to the entry file. */
					} else if (hasAttribute(label, ExternLabel) == SUCCESS) {
						if (outputExt == NULL) { /* If that file was not created yet then it would be created. */
							outputExt = fopen(extFileName, "w+"); /* Creating\recreating the output file. */
							if (outputExt == NULL)
								errFatal(); /* should not happen but, just in case. */
						}
						writePlain(outputExt, getSymbol(label), address); /* Writing to the extern file. */
					}
					assembleJ(outputObj, address, operator, 0, getAddress(label)); /* Assembling the line with a label. */
				} else
					assembleJ(outputObj, address, operator, 1, rs); /* Assembling the line with a register. */
			}
			address += assembledLineSize; /* Updating the code address tracker, every line takes exactly 4 bytes. */
		} else { /* At this point the line can only be a data instruction line. */
			instructor = searchInstructorByString(word); /* Getting the instructor. */
			expecting = getExpectation(instructor); /* To know what should be the next part of the line. */
			if (expecting == ExpectString) {
				status = getAscizParam(sourceLine, &expecting, &index, &symbol); /* Extracting the string. */
				if (status == HardwareErrorFlag) {
					errFatal(); /* Memory allocation had failed, cannot continue the program. */
				}
				assembleAsciz(dataSegment, symbol, &dataSegmentIndex); /* Copying the string to the data segment, it will be added to the output file at the end. */
				free(symbol); /* Avoiding memory leaks. */
				symbol = NULL; /* Protecting other if statements. */
			} else if (expecting == Expect8BitParams || expecting == Expect16BitParams || expecting == Expect32BitParams) { /* The instructor is db, dh, or dw. */
				count = 0; /* Initializing the argument counting variable. */
				sizeExpectation = expecting; /* Keeping that expectation for the assembling part since getDataParam will modify it. */
				getDataParam(sourceLine, &expecting, &index, &count, &args); /* Extracting the arguments. */
				assembleData(dataSegment, &dataSegmentIndex, sizeExpectation, count, args); /* Copying the argument to the data segment. */
				free(args); /* Avoiding memory leak. */
				args = NULL; /* Good practice. */
			}
		}
		free(word); /* freeing the word. */
	}

	dataSegmentIndex = 0; /* Starting from the beginning of the code segment. */
	/* Copying all the data segment into the output file. */
	while (dataSegmentIndex < dc) {
		/* Writing every character in hexadecimal format. */
		fprintf(outputObj, "%02X", dataSegment[dataSegmentIndex++]);
		address++; /* Incrementing the total address. */
		if (address % assembledLineSize == 0) 
			/* Every 4 bytes, printing the address in decimal format. */
			fprintf(outputObj, "\n%04ld ", address);
	}

	/* Freeing memory. */
	free(obFileName);
	free(entFileName);
	free(extFileName);
	free(dataSegment);
	/* Closing used file streams. */
	fclose(outputObj);
	if (outputEnt != NULL)
		fclose(outputEnt);
	if (outputExt != NULL)
		fclose(outputExt);
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

/**
 * Assembles the given data into the given stream.
 * This function writes the opcode of the given R operator then the
 * given registers and then the funct value of that operator as a
 * bit field into the given stream, in 8 bit sections separated with
 * spaces with the given address at the beginning of the line and
 * a new line character at the end of the line.
 * Expects the given register values to not be larger than 5 bits as
 * well as the funct value of the given operator, also the opcode of
 * the given operator should not require more than 6 bits.
 */
void assembleR(FILE *output, unsigned long int address, Operator *operator, char rs, char rt, char rd) {
	const char unusedBits = 6; /* The size of the unused part in the bit field. */
	const char registerDiffBits = 5; /* The size of the registers in the bit field. */
	const char bitSectionSize = 8; /* The size of every section in the bit field. */
	unsigned long int data = 0; /* The bit field. */
	unsigned char bitSection; /* To hold divided sections from the bit field. */

	data += getOpcode(operator); /* Inserting the opcode into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += rs; /* Inserting the first register into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += rt; /* Inserting the second register into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += rd; /* Inserting the third register into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += getFunct(operator); /* Inserting the funct value into the bit field. */
	data <<= unusedBits; /* Shifting the field 6 bits. */

	/* Writing the memory address into the output file. */
	fprintf(output, "%04ld ", address);

	/* The size of the bit field is (at least) 32 bits. */
	bitSection = data; /* Extracting the first 8 bits from the bit field */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the last 8 bits. */
	fprintf(output, "%02X\n", bitSection); /* Writing them into the output file in hexadecimal format. */
}

/**
 * Assembles the given data into the given stream.
 * This function writes the opcode of the given I operator then the
 * given registers and lastly the immediate value as a bit field
 * into the given stream. The bit field is written between the
 * given address and a new line character and is divided into 8 bit
 * sections separated by spaces.
 * Expects the given register values to not require more than 5
 * bits.
 */
void assembleI(FILE *output, unsigned long int address, Operator *operator, char rs, char rt, short immed) {
	const char registerDiffBits = 5; /* The size of the registers in the bit field. */
	const char immediateDiffBits = 16; /* The size of the immediate value in the bit field. */
	const char bitSectionSize = 8; /* The size of every section in the bit field. */
	unsigned long int data = 0; /* The bit field. */
	unsigned char bitSection; /* To hold divided sections from the bit field. */

	data += getOpcode(operator); /* Inserting the opcode into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += rs; /* Inserting the first register into the bit field. */
	data <<= registerDiffBits; /* Shifting the field 5 bits. */
	data += rt; /* Inserting the second register into the bit field. */
	data <<= immediateDiffBits; /* Shifting the field 16 bits. */
	data += immed; /* Inserting the immediate value into the bit field. */

	/* Writing the memory address into the output file. */
	fprintf(output, "%04ld ", address);

	/* The size of the bit field is (at least) 32 bits. */
	bitSection = data; /* Extracting the first 8 bits from the bit field */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the last 8 bits. */
	fprintf(output, "%02X\n", bitSection); /* Writing them into the output file in hexadecimal format. */
}

void assembleJ(FILE *output, unsigned long int address, Operator *operator, char isRegister, unsigned long int addressValue) {
	const char addressDiffBits = 24; /* The size of the address in the bit field. */
	const char bitSectionSize = 8; /* The size of every section in the bit field. */
	unsigned long int data = 0; /* The bit field. */
	unsigned char bitSection; /* To hold divided sections from the bit field. */

	data += getOpcode(operator); /* Inserting the opcode into the bit field. */
	data <<= 1; /* Shifting the bit field 1 bit for the isRegister parameter. */
	data += isRegister; /* Inserting the isRegister parameter into the bit field. */
	data <<= addressDiffBits; /* Shifting the bit field 24 bit to the left for the address value. */
	data += addressValue; /* Inserting the address value. */

	/* Writing the memory address into the output file. */
	fprintf(output, "%04ld ", address);

	/* The size of the bit field is (at least) 32 bits. */
	bitSection = data; /* Extracting the first 8 bits from the bit field */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the next 8 bits. */
	fprintf(output, "%02X ", bitSection); /* Writing them into the output file in hexadecimal format. */
	data >>= bitSectionSize; /* Shifting the field 8 bits to the right. */
	bitSection = data; /* Then extracting the last 8 bits. */
	fprintf(output, "%02X\n", bitSection); /* Writing them into the output file in hexadecimal format. */
}

/**
 * Copies the data from the second parameter into the first
 * parameter while incrementing the index pointed by the
 * the third parameter. This function copies the given string
 * into the given data segment, null character included.
 */
void assembleAsciz(char *dataSegment, char *str, unsigned long int *startIndex) {
	const char nullTermination = '\0'; /* Null terminating character constant. */
	while (*str != nullTermination) {
		/* Copying all characters from the string to the data segment. */
		dataSegment[*startIndex] = *str;
		(*startIndex)++; /* Incrementing the index. */
		str++;
	}
	/* Adding a null terminating character at the end (not necessarily the end of the data segment). */
	dataSegment[*startIndex] = nullTermination;
	(*startIndex)++; /* Incrementing the index. */
}

/**
 * Copies the data from the lats parameter into the first
 * parameter while incrementing the index pointed by the
 * the second parameter. This function copies the given
 * argument array into the given data segment, the size
 * of each argument is determined by the third parameter.
 */
void assembleData(char *dataSegment, unsigned long int *startIndex, const Expectation expecting, const int count, void *args) {
	const char byteSize = 8; /* Used for shifting every argument. */
	int argsIndex = 0; /* To track the given arguments array. */
	long int argument; /* To hold every argument. */
	while (argsIndex < count) {
		argument = ((long int*)args)[argsIndex]; /* Extracting the argument as a bit field. */

		dataSegment[(*startIndex)++] = argument; /* Extracting the first byte. */
		if (expecting == Expect16BitParams) { /* If the argument is larger one more byte would be extracted. */
			dataSegment[(*startIndex)++] = (argument >> byteSize);
		} else if (expecting == Expect32BitParams) { /* If the argument is even larger 3 more bytes would be extracted. */
			dataSegment[(*startIndex)++] = (argument >>= byteSize);
			dataSegment[(*startIndex)++] = (argument >>= byteSize);
			dataSegment[(*startIndex)++] = (argument >> byteSize);
		}
	}
}

/**
 * Used for writing external and entry labels into
 * the given stream.
 * Simply writes the given symbol as a string followed
 * by the given address and a new line character.
 */
void writePlain(FILE *output, char *symbol, unsigned long int address) {
	fprintf(output, "%s %04ld\n", symbol, address);
}
