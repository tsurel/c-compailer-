#include <stdio.h>

#include "functions.h"
#include "utils.h"
#include "symboltable.h"
#include "errorhandler.h"

/* __Temporary test code__ */
/* __The following just prints the first character of every line__ */
void assemble(FILE *file) {
	char c;
	int i;
	SymbolTable *symbol = createSymbolTable("blah", 166);

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
