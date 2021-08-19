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

#define MAX_LABEL_SIZE 31 /* The maximum number of characters allowed in a symbol. */
#define MEMORY_START_ADDRESS 100 /* The memory address from which the program should be loaded. */

/**
 * The following functions should not be used outside this translation unit.
 */
Code checkSymbolTabel(const char *fileName, SymbolTable *symbolTable, Code code);
Code map(FILE *file, const char *fileName, SymbolTable **symboltable, char *sourceLine, unsigned long int *ic, unsigned long int *dc);
void increaseDataCounterByData(unsigned long int *dc, const int count, Expectation sizeExpectation);
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, char *sourceLine, const unsigned long int ic, const unsigned long int dc);
void extractOutputFileNames(const char *sourceFileName, char *obFileName, char *entFileName, char *extFileName);
void assembleR(FILE *output, unsigned long int address, Operator *op, char rs, char rt, char rd);
void assembleI(FILE *output, unsigned long int address, Operator *op, char rs, char rt, short immed);
void assembleJ(FILE *output, unsigned long int address, Operator *op, char isRegister, unsigned long int addressValue);
void assembleAsciz(char *dataSegment, char *str, unsigned long int *startIndex);
void assembleData(char *dataSegment, unsigned long int *startIndex, const Expectation expecting, const int count, long int *args);
void writePlain(FILE *output, char *symbol, unsigned long int address);
void writeDataSegment(FILE *output ,const unsigned long int dc, char *dataSegment, unsigned long int address);

/**
 * Takes in an assembly source file as a stream and assembles
 * it after checking if it has any issues. The file is assembled
 * only if it has no issues. If the file contains invalid syntax
 * or any other assembly related problem it would not be
 * assembled and error messages would be printed for the user
 * to see. The error messages can tell the user what are the
 * issues with his code.
 */
void assemble(FILE *sourceFile, const char *fileName) {
	unsigned long int ic = MEMORY_START_ADDRESS; /* Operator line counter (instruction counter). */
	unsigned long int dc = 0; /* Data instruction counter (data counter). */
	Code code; /* To track if output file should be created. */
	SymbolTable *symbolTable, *edit; /* Symbol table variables, the first is to point to the symbol table and the second is to point to a specific label. */
	char *sourceLine = malloc(SOURCE_LINE_LENGTH + 1); /* A pointer to every source line, used for scanning the file line by line. */

	if (sourceLine == NULL)
		errFatal(); /* Cannot continue without memory for the line. */

	/* Mapping the source file for labels and errors. */
	code = map(sourceFile, fileName, &symbolTable, sourceLine, &ic, &dc);

	edit = symbolTable; /* Starting from the first label */
	while (edit != NULL) { /* Looping through all of the labels in the symbol table. */
		if (hasAttribute(edit, DataLabel) == SUCCESS)
			setAddress(edit, getAddress(edit) + ic); /* All data labels should be positioned after the code segment. */
		edit = getNext(edit); /* Getting the next label. */
	}

	/* Looking for undeclared labels. */
	code = checkSymbolTabel(fileName, getNext(symbolTable), code); /* Ignoring the first impossible initializing label. */

	if (code == SUCCESS) { /* If the source file had no issues it can be assembled. */
		rewind(sourceFile); /* Preparing to re-scan the file from the beginning. */
		convert(sourceFile, fileName, symbolTable, sourceLine, ic - MEMORY_START_ADDRESS, dc); /* Creating the output files. */
	}

	/* Freeing the memory. */
	free(sourceLine);
	freeSymbolTable(symbolTable);
}

/**
 * Scans the given symbol table for used labels that
 * are undeclared. If such label was found an error
 * message would be printed for every one and an error
 * code would be returned. If no such label was found
 * a success code would be returned instead.
 */
Code checkSymbolTabel(const char *fileName, SymbolTable *symbolTable, Code code) {
	while (symbolTable != NULL) {
		if (isDeclared(symbolTable) == ERROR && hasAttribute(symbolTable, ExternLabel) == ERROR) { /* Checking the label. */
			errUndeclaredLabel(fileName, symbolTable); /* Printing an error message, the label is not fine. */
			code = ERROR; /* Setting the return value to error. */
		}
		symbolTable = getNext(symbolTable); /* Checking the next label. */
	}
	return code;
}

/**
 * Maps the given source file for labels and errors.
 * This function initializes the given symbol table
 * and assigns to it all the labels found in the
 * given source files. In addition this function
 * check that the given source file is valid
 * meaning, it has no syntax errors or other similar
 * issues and it can be assembled.
 * Also this function counts the size of the code
 * segment and data segment via the last two
 * parameters and prints out a message for the user
 * for every error found in the source file.
 * Returns an error code that indicates if the
 * given file should be assembled (meaning it has no
 * issues). If an assembled output file(s) should be
 * created this function would return SUCCESS and
 * if no assembled output files should be created
 * ERROR would be returned instead.
 * Note: the given symbol table is initialized to an
 * impossible label that should be ignored.
 */
Code map(FILE *file, const char *fileName, SymbolTable **symbolTable, char *sourceLine, unsigned long int *ic, unsigned long int *dc) {
	const char codeLineSize = 4; /* The size of an assembled code line, used for address tracking. */
	const char *stopOperator = "stop"; /* Special case keyword, no operands. */
	const char *jmpOperator = "jmp"; /* Special case keyword, the only J operator that can receive a register as operand. */
	const char beginLabelArgSetI = 15; /* The opcode of the I operator from which a label operand is required. */
	const char endLabelArgSetI = 18; /* The opcode of the I operator until which a label operand is required. */
	const char nullTermination = '\0', space = ' ', tab = '\t'; /* Syntax characters. */
	Code code = SUCCESS; /* Error code to track issues. */
	char shouldStop = 0; /* To track when the loop should stop meaning, the source file has ended */
	char isLabelLine; /* To track if there was a label at the beginning of the line. */
	int index; /* An index to track the position on the line. */
	unsigned long int lineNum = 0; /* To track the line number. */
	char *word; /* A variable to store the labels\Instructors\Operators returned from getWord. */
	char *symbol; /* A variable to store the label operand of I\J operators. */
	char *str; /* A variable to store the string returned from "getAscizParam" function from asmutils. */
	long int *args; /* To use the "getDataParam" function from asmutils. */
	char rs, rt, rd; /* Variables to use some of the "get" functions from asmutils. */
	short immed = 0; /* A variable to use the "getIParam" function from asmutils. */
	int count; /* Used for counting arguments for db and dh and dw data instructors. */
	char isLabeledArgSet = 0; /* To track I\J operators required argument sets. */
	Operator *operator; /* To hold operators. */
	Instructor *instructor; /* To hold instructors. */
	SymbolTable *front; /* The symbol table. */
	SymbolTable *edit; /* Used for editing the symbol table. */
	Expectation expecting; /* To differentiate different situations and catch issues. */
	Expectation dataExpectation; /* Used for holding the expectation of a data instructor. */
	Flag status; /* To differentiate different situations and catch issues. */

	if ((edit = front = addSymbol(NULL, "!", 0)) == NULL) /* Initializing the symbol table with an impossible label. */
		errFatal(); /* Memory allocation failed, cannot continue the program. */

	if ((word = malloc(MAX_LABEL_SIZE + 1)) == NULL || /* +1 for a terminating character. */
		(symbol = malloc(MAX_LABEL_SIZE + 1)) == NULL || /* +1 for a terminating character. */
		(str = malloc(SOURCE_LINE_LENGTH)) == NULL || /* An asciz string cannot be longer than that. */
		(args = calloc(sizeof(long int) ,(SOURCE_LINE_LENGTH / 2) + 1)) == NULL) /* A line of db or dh or dw will never have more arguments than that. */
	errFatal(); /* Cannot continue without memory. */

	while (!shouldStop) {
		isLabelLine = 0; /* The line is not labeled. */
		index = -1; /* Using this variable as length check for extractSourceLine. */
		rs = rt = rd = 0; /* Avoiding potential issues on "if" statement. */
		lineNum++; /* This is a new line. */

		if ((status = extractSourceLine(file, sourceLine, &index)) == EndFileFlag)
			shouldStop = 1; /* This is the last line in the source file. */
		if (errCheckLine(fileName, sourceLine, lineNum, index, status) == EEvent) { /* Checking and handling source file issues. */
			code = ERROR; /* No output should be created for this source file. */
			continue; /* The line is corrupted. */
		}
		index = 0; /* Setting the index to the beginning of the line. */

		if ((status = getWord(sourceLine, &expecting, &index, word)) == LabelFlag) { /* If the returned flag is LabelFlag then there are no errors to check for. */
			if (errCheckSymbol(front, fileName, sourceLine, word, lineNum) == EEvent) { /* Checking the symbol. */
				code = ERROR; /* No output should be created for this source file. */
				continue; /* The line is corrupted. */
			}
			if ((edit = searchLabel(front, word)) == NULL)
				if ((edit = addSymbol(front, word, 0)) == NULL) /* Creating the next label if it is not in the symbol table. */
					errFatal(); /* Memory allocation for this label had failed, cannot continue the program. */
			isLabelLine = 1; /* The line is labeled. */

			status = getWord(sourceLine, &expecting, &index, word); /* Extracting the next part of the source line. */
		} else if (status == CommentLineFlag)
			continue; /* Skipping a comment line. */
		if (status == OperatorFlag && expecting == ExpectWord) { /* This combination indicates that the line is empty. */
			if (isLabelLine) {
				errLonelyLabel(fileName, sourceLine, lineNum); /* A label cannot be alone in a line. */
				code = ERROR; /* No output should be created for this source file. */
			}
			continue; /* The line is corrupted or empty. */
		}
		if (errCheckWord(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) { /* Checking and handling source file issues. */
			code = ERROR; /* No output should be created for this source file. */
			continue; /* The line is corrupted. */
		}

		/* Beginning arguments scanning. */
		if (status == OperatorFlag) { /* The word is an operator. */
			if (isLabelLine) {
				addAttribute(edit, CodeLabel); /* Previous checks prevent this from failing. */
				setAddress(edit, *ic); /* Stetting the address of this label. */
			}
			operator = searchOperatorByString(word); /* Getting the operator. */
			if (operator == NULL) {
				errInvalidKeyword(fileName, sourceLine, word, lineNum); /* The operator is invalid. */
				code = ERROR; /* No output should be created for this source file. */
				continue; /* The line is corrupted. */
			}
			(*ic) += codeLineSize;
			if (getType(operator) == R) { /* Handling R type operators. */
				if (getOpcode(operator))
					/* Extracting 2 operands. */
					status = getRParam(sourceLine, &expecting, R2, &index, &rs, &rt, &rd);
				else
					/* Extracting 3 operands. */
					status = getRParam(sourceLine, &expecting, R3, &index, &rs, &rt, &rd);
				if (errCheckR(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) { /* Checking and handling source file issues. */
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
			} else if (getType(operator) == I) { /* Handling I type operators. */
				/* Extracting the data from the line as operand set for I operators. */
				status = getIParam(sourceLine, &expecting, &index, &rs, &rt, &immed, &isLabeledArgSet, symbol);
				if (errCheckI(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) { /* Checking and handling source file issues. */
					code = ERROR;
					continue;
				}
				if (isLabeledArgSet) { /* One of the operands is a label. */
					if (getOpcode(operator) < beginLabelArgSetI || getOpcode(operator) > endLabelArgSetI) { /* Checking if this is a valid argument set. */
						errInvalidArgumentSet(fileName, sourceLine, lineNum, I, 0); /* The argument set is invalid. */
						code = ERROR; /* No output should be created for this source file. */
						continue; /* The line is corrupted. */
					}
					if (errCheckSymbol(NULL, fileName, sourceLine, symbol, lineNum) == EEvent) { /* Checking if the symbol is a reserved keyword. */
						code = ERROR; /* No output should be created for this source file. */
						continue; /* The line is corrupted. */
					}
					if (searchLabel(front, symbol) == NULL) /* Extracting the label. */
						if (addSymbol(front, symbol, lineNum) == NULL) /* Line number as address for error messaging purposes. */
							errFatal(); /* Memory allocation for this label had failed, cannot continue the program. */
				} else { /* There is no label, the middle operand is an immediate value. */
					if (getOpcode(operator) >= beginLabelArgSetI && getOpcode(operator) <= endLabelArgSetI) { /* Checking if this is a valid argument set. */
						errInvalidArgumentSet(fileName, sourceLine, lineNum, I, 1); /* The argument set is invalid. */
						code = ERROR; /* No output should be created for this source file. */
						continue; /* The line is corrupted. */
					}
				}
			} else if (strcmp(getOperatorKeyword(operator), stopOperator) != 0) { /* The remaining operators must be of type J. */
				/* Extracting the data from the line. */
				status = getJParam(sourceLine, &expecting, &index, &rs, &isLabeledArgSet, symbol);
				if (errCheckJ(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) { /* Checking and handling source file issues. */
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				if (isLabeledArgSet) { /* The operand is a label. */
					if (errCheckSymbol(NULL, fileName, sourceLine, symbol, lineNum) == EEvent) { /* Checking if the symbol is a reserved keyword. */
						code = ERROR; /* No output should be created for this source file. */
						continue; /* The line is corrupted. */
					}
					if (searchLabel(front, symbol) == NULL) /* Extracting the label. */
						if (addSymbol(front, symbol, lineNum) == NULL) /* Line number as address for error messaging purposes. */
							errFatal(); /* Memory allocation for this label had failed, cannot continue the program. */
				} else { /* The operand is a register */
					if (strcmp(getOperatorKeyword(operator), jmpOperator) != 0) { /* the "jmp" operator is the only one that can take a register as operand. */
						errInvalidArgumentSet(fileName, sourceLine, lineNum, J, 0);
						code = ERROR; /* No output should be created for this source file. */
						continue; /* The line is corrupted. */
					}
				}
			} /* Special case, the "stop" keyword, expecting no operands. */
		} else if (status == InstructorFlag) { /* The word is a data instructor. */
			if (isLabelLine) {
				addAttribute(edit, DataLabel); /* Previous checks prevent this from failing. */
				setAddress(edit, *dc); /* Stetting the address of this label. */
			}
			instructor = searchInstructorByString(word); /* Getting the instructor. */
			if (instructor == NULL) {
				errInvalidKeyword(fileName, sourceLine, word, lineNum); /* The instructor is invalid. */
				code = ERROR; /* No output should be created for this source file. */
				continue; /* The line is corrupted. */
			}
			dataExpectation = getExpectation(instructor); /* Saving the expectation. */
			if (dataExpectation == ExpectString) { /* This is an asciz data instructor. */
				status = getAscizParam(sourceLine, &expecting, &index, str);
				if (errCheckAsciz(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) {
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				/* Adding the size of the string to the data counter. */
				(*dc) += strlen(str) + 1; /* +1 for a terminating character. */
			} else if (dataExpectation == ExpectLabelEntry) { /* This is an entry instructor. */
				if (isLabelLine) { /* Unnecessary label at the beginning of the line. */
					wrnLabeledLine(fileName, sourceLine, lineNum, dataExpectation); /* Printing a warning message. */
					removeSymbol(&front, edit); /* The assembler will ignore this label. */
				}
				/* Extracting the label from the line. */
				status = getWord(sourceLine, &expecting, &index, symbol);
				/* Checking and handling source file issues. */
				if (errCheckExpectLabel(fileName, sourceLine, symbol, lineNum, index, expecting, status) == EEvent ||
					errCheckSymbol(NULL, fileName, sourceLine, symbol, lineNum) == EEvent) { /* Checking the symbol. */
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				if ((edit = searchLabel(front, symbol)) == NULL)
					if ((edit = addSymbol(front, symbol, 0)) == NULL) /* Creating the next label if it is not in the symbol table. */
						errFatal(); /* Memory allocation for this label had failed, cannot continue the program. */
				if (hasAttribute(edit, ExternLabel) == SUCCESS) { /* A label cannot be both entry and external. */
					errBothEntryAndExtern(fileName, sourceLine, lineNum, dataExpectation);
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				addAttribute(edit, EntryLabel); /* This is an entry label, previous checks prevent this from failing */
				if (isDeclared(edit) == ERROR)
					setAddress(edit, lineNum); /* If the label is not declared it should be ready for an error message. */
			} else if (dataExpectation == ExpectLabelExternal) { /* This is an extern instructor. */
				if (isLabelLine) { /* Unnecessary label at the beginning of the line. */
					wrnLabeledLine(fileName, sourceLine, lineNum, dataExpectation); /* Printing a warning message. */
					removeSymbol(&front, edit); /* The assembler will ignore this label. */
				}
				/* Extracting the label from the line. */
				status = getWord(sourceLine, &expecting, &index, symbol);
				/* Checking and handling source file issues. */
				if (errCheckExpectLabel(fileName, sourceLine, symbol, lineNum, index, expecting, status) == EEvent ||
					errCheckSymbol(NULL, fileName, sourceLine, symbol, lineNum) == EEvent) { /* Checking the symbol. */
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				if ((edit = searchLabel(front, symbol)) == NULL)
					if ((edit = addSymbol(front, symbol, 0)) == NULL) /* Creating the next label if it is not in the symbol table. */
						errFatal(); /* Memory allocation for this label had failed, cannot continue the program. */
				if (hasAttribute(edit, EntryLabel) == SUCCESS) { /* A label cannot be both entry and external. */
					errBothEntryAndExtern(fileName, sourceLine, lineNum, dataExpectation);
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				if (isDeclared(edit) == SUCCESS) { /* An external label cannot be declared locally. */
					errDeclaredExtern(fileName, sourceLine, getSymbol(edit), lineNum); /* Printing relevant error message. */
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				addAttribute(edit, ExternLabel); /* This is an external label, previous checks prevent this from failing */
				setAddress(edit, 0); /* External labels have no address. */
			} else if (dataExpectation == Expect8BitParams || dataExpectation == Expect16BitParams || dataExpectation == Expect32BitParams) {
				expecting = dataExpectation; /* The "getDataParam" function requires the expectation. */
				status = getDataParam(sourceLine, &expecting, &index, &count, args); /* Extracting the arguments. */
				if (errCheckData(fileName, sourceLine, lineNum, index, expecting, status) == EEvent) {
					code = ERROR; /* No output should be created for this source file. */
					continue; /* The line is corrupted. */
				}
				increaseDataCounterByData(dc, count, dataExpectation); /* Incrementing the data counter based on the instruction and the number of arguments. */
			}
		}
		while (sourceLine[index] != nullTermination) {/* Checking for unexpected characters that might have bean missed by some "get" functions from asmutils. */
			if (sourceLine[index] != space && sourceLine[index] != tab) {
				errUnexpectedToken(fileName, sourceLine, lineNum, index);
				code = ERROR; /* No output should be created for this source file. */
				break; /* One message like this per line is enough. */
			}
		index++; /* Incrementing the index. */
		}
	}

	*symbolTable = front; /* Returning the symbol table trough a parameter. */

	/* Avoiding memory leak. */
	free(word);
	free(symbol);
	free(str);
	free(args);

	return code;
}

/**
 * Increments the given data counter based on the size of
 * the arguments determined by the last parameter.
 */
void increaseDataCounterByData(unsigned long int *dc, const int count, Expectation sizeExpectation) {
	const char halfSize = 2; /* The size of every dh argument. */
	const char wordSize = 4; /* The size of every dw argument. */
	if (sizeExpectation == Expect8BitParams)
		(*dc) += count; /* Increment for db. */
	else if (sizeExpectation == Expect16BitParams)
		(*dc) += (count * halfSize); /* Increment for dh. */
	else
		(*dc) += (count * wordSize); /* Increment for dw. */
}

/**
 * Assembles the given source file into output files
 * created by this function. This function expects
 * the given source file to contain no issues and the
 * given symbol table to be initialized and set with
 * all labels from the file in it. In addition the
 * last two parameters are expected to equal the size
 * of the code segment and the size of the data
 * segment respectively.
 * This function will create an .ob file for all the
 * assembled data and .ent or .ext files if the
 * source file contains entry or external labels
 * respectively.
 */
void convert(FILE *file, const char *fileName, SymbolTable *symboltable, char *sourceLine, const unsigned long int ic, const unsigned long int dc) {
	const char assembledLineSize = 4; /* The size for the bit field in the output file. */
	const char *stopOperator = "stop"; /* Special case keyword, no operands. */
	int index; /* An index to track the position on the line. */
	int lengthCheck = -1; /* A variable to use the extractSourceLine function. */
	int count; /* Used for counting arguments for db, dh, and dw keywords. */
	unsigned long int address = MEMORY_START_ADDRESS; /* To track the memory address of the assembled operators in the output file. */
	char shouldStop = 0; /* To track when the loop should stop meaning, the source file has ended */
	char isLabeledArgSet = 0; /* To use the "getIParam" and "getJParam" functions from asmutils. */
	char *word; /* A variable to store the labels\Instructors\Operators returned from getWord. */
	char *symbol; /* A variable to store the label operand of I\J operators. */
	char *str; /* A variable to store and access asciz strings. */
	long int *args; /* To store and access db\dh\dw arguments. */
	char rs, rt, rd; /* Variables to store register addresses. */
	short immed; /* A variable to store the immediate value for I operators. */
	char *dataSegment; /* Points to the array that stores all the assembled data instructors parameters. */
	unsigned long int dataSegmentIndex = 0; /* Index variable for the data segment array. */
	FILE *outputObj; /* The main output file. */
	FILE *outputEnt = NULL, *outputExt = NULL; /* Entries and externals output files. */
	char *obFileName, *entFileName, *extFileName; /* pointers to the names of the output files. */
	SymbolTable *label; /* A variable for label handling. */
	Operator *operator; /* To hold operators. */
	Instructor *instructor; /* To hold instructors. */
	Expectation expecting; /* To use functions and track data instruction expectation. */
	Expectation sizeExpectation; /* Used for extracting data arguments. */
	Flag status; /* To differentiate different situations and catch memory allocation issues. */


	/* Allocating memory for the strings that should store the output file names. */
	if ((obFileName = malloc(strlen(fileName) - FILE_EXTENSION_LEN + strlen(OUTPUT_OB_EXTENTION) + 1)) == NULL)
		errFatal(); /* Cannot continue without memory. */
	if ((entFileName = malloc(strlen(fileName) - FILE_EXTENSION_LEN + strlen(OUTPUT_ENT_EXTENTION) + 1)) == NULL)
		errFatal(); /* Cannot continue without memory. */
	if ((extFileName = malloc(strlen(fileName) - FILE_EXTENSION_LEN + strlen(OUTPUT_EXT_EXTENTION) + 1)) == NULL)
		errFatal(); /* Cannot continue without memory. */

	/* Getting the names of the output files. */
	extractOutputFileNames(fileName, obFileName, entFileName, extFileName);

	outputObj = fopen(obFileName, "w+"); /* Creating/recreating the output file. */
	if (outputObj == NULL)
		errFatal(); /* cannot continue without the output file. */
	if ((dataSegment = malloc(dc)) == NULL) { /* Allocating memory for the data segment. */
		errFatal(); /* Cannot continue without memory. */
	}

	if ((word = malloc(MAX_LABEL_SIZE + 1)) == NULL || /* +1 for a terminating character. */
		(symbol = malloc(MAX_LABEL_SIZE + 1)) == NULL || /* +1 for a terminating character. */
		(str = malloc(SOURCE_LINE_LENGTH)) == NULL || /* An asciz string cannot be longer than that. */
		(args = calloc(sizeof(long int) ,(SOURCE_LINE_LENGTH / 2) + 1)) == NULL) /* A line of db or dh or dw will never have more arguments than that. */
	errFatal(); /* Cannot continue without memory. */

	fprintf(outputObj, "     %ld %ld\n", ic, dc);

	while (!shouldStop) {
		index = 0; /* The line start at index 0. */
		rs = rt = rd = 0; /* Avoid problems in functions that use registers. */

		if ((status = extractSourceLine(file, sourceLine, &lengthCheck)) == EndFileFlag)
			shouldStop = 1; /* This is the last line in the source file. */

		if ((status = getWord(sourceLine, &expecting, &index, word)) == LabelFlag) { /* Extracting the beginning of the line. */
			status = getWord(sourceLine, &expecting, &index, word); /* Extracting again if it was a label. */
		} else if (status == CommentLineFlag || (status == OperatorFlag && expecting == ExpectWord))
			continue; /* Skipping a comment line or an empty line. */

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
				status = getIParam(sourceLine, &expecting, &index, &rs, &rt, &immed, &isLabeledArgSet, symbol);
				if (isLabeledArgSet) { /* If one of the operands is a label. */
					label = searchLabel(symboltable, symbol); /* Extracting the label. */
					immed = getAddress(label) - address; /* Calculating the difference into the immediate field. */
				} /* If there was no label no special treatment is required. */
				assembleI(outputObj, address, operator, rs, rt, immed); /* Assembling the line. */
			} else if (strcmp(word, stopOperator) == 0) { /* Special case, the "stop" keyword. */
				assembleJ(outputObj, address, operator, 0, 0); /* The "stop" keyword takes no operands. */
			} else { /* The remaining operators must be of type J. */
				/* Extracting the data from the line. */
				status = getJParam(sourceLine, &expecting, &index, &rs, &isLabeledArgSet, symbol);
				if (isLabeledArgSet) { /* If the operand is a label. */
					label = searchLabel(symboltable, symbol); /* Extracting the label from the symbol table. */
					if (hasAttribute(label, EntryLabel) == SUCCESS) { /* This may be an entry label. */
						if (outputEnt == NULL) { /* If that file was not created yet then it would be created. */
							outputEnt = fopen(entFileName, "w+"); /* Creating\recreating the output file. */
							if (outputEnt == NULL)
								errFatal(); /* should not happen but, just in case. */
						}
						writePlain(outputEnt, getSymbol(label), getAddress(label)); /* Writing to the entry file. */
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
				status = getAscizParam(sourceLine, &expecting, &index, str); /* Extracting the string. */
				assembleAsciz(dataSegment, str, &dataSegmentIndex); /* Copying the string to the data segment, it will be added to the output file at the end. */
			} else if (expecting == Expect8BitParams || expecting == Expect16BitParams || expecting == Expect32BitParams) { /* The instructor is db, dh, or dw. */
				count = 0; /* Initializing the argument counting variable. */
				sizeExpectation = expecting; /* Keeping that expectation for the assembling part since getDataParam will modify it. */
				getDataParam(sourceLine, &expecting, &index, &count, args); /* Extracting the arguments. */
				assembleData(dataSegment, &dataSegmentIndex, sizeExpectation, count, args); /* Copying the argument to the data segment. */
			}
		}
	}

	writeDataSegment(outputObj, dc, dataSegment, address); /* Writing the data segment to the output file. */

	/* Freeing memory. */
	free(obFileName);
	free(entFileName);
	free(extFileName);
	free(dataSegment);
	free(word);
	free(symbol);
	free(str);
	free(args);
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
 * This function modifies the last three buffers.
 * This is a private utility function for the convert function.
 */
void extractOutputFileNames(const char *sourceFileName, char *obFileName, char *entFileName, char *extFileName) {
	const char nullTermination = '\0'; /* Null terminating character, used for string handling. */
	char *srcFileName; /* To contain the source file name without the extension. */
	int sourceFileNameLen = strlen(sourceFileName); /* Used in multiple locations. */

	/* Allocating memory for the extension-less file name buffer. */
	if ((srcFileName = malloc(sourceFileNameLen - FILE_EXTENSION_LEN)) == NULL)
		errFatal(); /* Cannot continue without memory. */

	/* Extracting the source file's name without the extension into srcFileName. */
	subString(srcFileName, sourceFileName, 0, sourceFileNameLen - FILE_EXTENSION_LEN - 1);
	srcFileName[sourceFileNameLen - FILE_EXTENSION_LEN] = nullTermination; /* Avoiding issues with strcat. */

	/* Assigning a terminating character at the beginning of each buffer before using strcat. */
	obFileName[0] = nullTermination;
	entFileName[0] = nullTermination;
	extFileName[0] = nullTermination;

	/* Copying the name of the file without the extension into the buffers. */
	strcat(obFileName, srcFileName);
	strcat(entFileName, srcFileName);
	strcat(extFileName, srcFileName);

	/* Assigning the right extension for every output file name. */
	strcat(obFileName, OUTPUT_OB_EXTENTION);
	strcat(entFileName, OUTPUT_ENT_EXTENTION);
	strcat(extFileName, OUTPUT_EXT_EXTENTION);

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
	data += ((unsigned short)immed); /* Inserting the immediate value into the bit field. */

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
	const char addressDiffBits = 25; /* The size of the address in the bit field. */
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
void assembleData(char *dataSegment, unsigned long int *startIndex, const Expectation expecting, const int count, long int *args) {
	const char byteSize = 8; /* Used for shifting every argument. */
	int argsIndex = 0; /* To track the given arguments array. */
	long int argument; /* To hold every argument. */
	while (argsIndex < count) {
		argument = args[argsIndex]; /* Extracting the argument as a bit field. */

		dataSegment[(*startIndex)++] = argument; /* Extracting the first byte. */
		if (expecting == Expect16BitParams) { /* If the argument is larger one more byte would be extracted. */
			dataSegment[(*startIndex)++] = (argument >> byteSize);
		} else if (expecting == Expect32BitParams) { /* If the argument is even larger 3 more bytes would be extracted. */
			dataSegment[(*startIndex)++] = (argument >>= byteSize);
			dataSegment[(*startIndex)++] = (argument >>= byteSize);
			dataSegment[(*startIndex)++] = (argument >> byteSize);
		}
		argsIndex++;
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



/**
 * Writes the given data segment to the given output stream.
 */
void writeDataSegment(FILE *output ,const unsigned long int dc, char *dataSegment, unsigned long int address) {
	const char assembledLineSize = 4; /* The size for the bit field in the output file. */
	int index = 0; /* Starting from the beginning of the data segment. */

	if (dc == 0)
		return; /* If the data segment is empty then there is nothing to write. */

	fprintf(output, "%04ld", address); /* Writing the address for the first line in the loop. */
	/* Copying all the data segment into the output file. */
	while (index < dc) {
		/* Writing every character in hexadecimal format. */
		fprintf(output, " %02X", ((unsigned char)dataSegment[index++]));
		address++; /* Incrementing the total address. */
		if (address % assembledLineSize == 0) 
			/* Every 4 bytes, printing the address in decimal format. */
			fprintf(output, "\n%04ld", address);
	}
}
