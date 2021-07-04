#include <stdio.h>

#include "converter.h"
#include "utils.h"
#include "symboltable.h"
#include "errorhandler.h"
#include "operations.h"

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
void assemble(FILE *file) {
	char c;
	int i;
	Operation *op;
	SymbolTable *symbol = createSymbolTable("blah", 166);
	initasmOperations();
	op = searchKeyword("add");
	printasm();
	printf("\n---%s\t%d\t%d\t%d\n", op->keyword, op->type, op->funct, op->opcode);
	if (searchKeyword("blah") == NULL)
		printf("%s\n", "SUCCESS");
	clearasmOperations();

	for (i = 0; i < 5; ++i) {
		if (addAttribute(symbol, CODE) == ERROR)
			printf("%s\n", "YAY");
	}

	for (i = 0; i < ATTRS_PER_LABEL; ++i) {
		printf("%d\n", symbol->attributes[i]);
	}

	printf("%s\t%d\n", symbol->symbol, symbol->address);
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			printf("%c", c);
		} else {
			printf("%c\n", c);
			skipln(file);
		}
	}
	printf("\n");
	freeSymbolTable(symbol);
}
