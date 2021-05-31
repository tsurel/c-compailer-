#include <stdio.h>
#include "functions.h"

/* __Temporary test code__ */
/* __The following just prints the given file to the console__ */
void assemble(FILE *file) {
	char c;
	while ((c = fgetc(file)) != EOF) {
		printf("%c", c);
	}
	printf("\n");
}
