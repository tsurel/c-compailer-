#include <stdlib.h>

#include "symboltable.h"
#include "errorhandler.h"

/**
 * Contains a collection of utility functions for the symbol
 * table data structure.
 */

/**
 * Creates a new symbol table node with the given symbol and address.
 * The pointer to the next node is initialized to null while every index
 * of the attributes array is marked as empty.
 * This function returns a pointer to the new symbol table node.
 */
SymbolTable *createSymbolTable(char *symbol, int address) {
	int i;
	SymbolTable *symbolTable = malloc(sizeof(SymbolTable));
	/*
	In case no memory could be allocated a null pointer will be returned
	instead.
	*/
	if (symbolTable == NULL) {
		return NULL;
	}

	symbolTable->symbol = symbol; /* Initializing the symbol. */
	symbolTable->address = address; /* Initializing the address. */
	symbolTable->next = NULL; /* Initializing the pointer to the next node. */

	/* Setting every index of the attributes array to null. */
	for (i = 0; i < ATTRS_PER_LABEL; ++i) {
		symbolTable->attributes[i] = EMPTY;
	}

	return symbolTable; /* Returning the address of the new symbol table node. */
}

/**
 * Adds the given attribute code to the given label.
 */
unsigned char addAttribute(SymbolTable *symbolTable, unsigned char attributeCode) {
	int i;
	for (i = 0; i < ATTRS_PER_LABEL; ++i) {
		if (symbolTable->attributes[i] == EMPTY) {
			symbolTable->attributes[i] = attributeCode;
			return SUCCESS;
		}
	}

	return ERROR;
}

void freeSymbolTable(SymbolTable *symbolTable) {
	free(symbolTable);
}
