#include <stdio.h>
#include <stdlib.h>

#include "converter.h"
#include "utils.h"
#include "symboltable.h"
#include "asmutils.h"
#include "operations.h"

/**
 * The converter translation unit is responsible for managing the assembling
 * process.
 */

/**
 * The following functions should not be used outside of this translation unit.
 */
Code map(FILE *file, SymbolTable *symboltable);
void convert(FILE *file, SymbolTable *symboltable);
char *assembleR(Operation *op, char rs, char rt, char rd);
char *assembleI(Operation *op, char rs, char rt, short immed);
char *assembleJ(Operation *op, unsigned char reg, unsigned address);
/* TODO: Implement the assembler itself using the functions above. */

/* __Temporary test code__ */
/* __The following just prints the first character of every line__ */
void test(FILE *file) {
	int start = 0, end = -1;
	char *str = malloc(SOURCE_LINE_LENGTH + 1);
	Expectation expecting = ExpectDollarSign;

	Flag flag;
	if (str == NULL)
		exit(EXIT_FAILURE);

	flag = extractSourceLine(file, str, &end);
	printf("\n");
	printf("%d\n", flag);
	printf("%d\n", end);
	flag = rangeRParam(str, &expecting, R3, &start, &end);
	printf("\n");
	printf("%s\n", str);
	printf("Flag: %d\n", flag);
	printf("Expectation: %d\n", expecting);
	printf("Range: %d --- %d\n", start, end);
	printf("\n");
	free(str);
}

void assemble(FILE *file) {
	test(file);
}

Code map(FILE *file, SymbolTable *symboltable) {
	return 0;
}

void convert(FILE *file, SymbolTable *symboltable) {

}
