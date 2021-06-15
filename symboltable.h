#pragma once

/**
 * An header file for the symbol table data structure.
 */

#define EMPTY    0 /* An empty attribute. */
#define CODE     1 /* Code attribute.     */
#define DATA     2 /* Data attribute.     */
#define ENTRY 	 3 /* Entry attribute.    */
#define EXTERNAL 4 /* External attribute. */
#define ATTRS_PER_LABEL 2 /* Maximum number of attributes per label. */

/**
 * Defining the symbol table structure in the form of a node chain.
 * This structure is used to map all labels in the assembly code.
 * Every node of this structure represents a label.
 */
typedef struct symbolt SymbolTable;
struct symbolt {
	char *symbol; /* Stores the symbol of that label. */
	int address; /* Stores the memory address of that label. */
	unsigned char attributes[ATTRS_PER_LABEL]; /* Stores the attributes codes of that label. */
	struct symbolt *next; /* A pointer to the next label. */
};

SymbolTable *createSymbolTable(char *symbol, int address);
unsigned char addAttribute(SymbolTable *symbolTable ,unsigned char attributeCode);
void freeSymbolTable(SymbolTable *symbolTable);
