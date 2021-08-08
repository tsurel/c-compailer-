#include <stdio.h>
#include <stdlib.h>

#include "converter.h"
#include "utils.h"
#include "symboltable.h"
#include "asmutils.h"
#include "keywords.h"

/**
 * The converter translation unit is responsible for managing the assembling
 * process.
 */

/**
 * The following functions should not be used outside of this translation unit.
 */
Code map(FILE *file, SymbolTable *symboltable);
void convert(FILE *file, SymbolTable *symboltable);
char *assembleR(Operator *op, char rs, char rt, char rd);
char *assembleI(Operator *op, char rs, char rt, short immed);
char *assembleJ(Operator *op, unsigned char reg, unsigned address);
/* TODO: Implement the assembler itself using the functions above. */

/* __Temporary test code__ */
void test(FILE *file) {
	Flag flag;
	Expectation expecting;
	int index = -1;
	char *line = malloc(SOURCE_LINE_LENGTH + 1);
	char rs = 0, rt = 0, rd = 0;

	if (line == NULL)
		exit(EXIT_FAILURE);

	flag = extractSourceLine(file, line, &index);
	printf("%s\n", line);

	index = 0;
	flag = getRParam(line, &expecting, R2, &index, &rs, &rt, &rd);

	printf("%d\t%d\t%d\n", flag, expecting, index);
	if (flag == NoIssueFlag && expecting == ExpectDigitOrEnd)
		printf("%d\t%d\t%d\n", rs, rt, rd);
}

void assemble(FILE *file) {
	test(file);
}

Code map(FILE *file, SymbolTable *symboltable) {
	return 0;
}

void convert(FILE *file, SymbolTable *symboltable) {

}
