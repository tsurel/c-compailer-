#include <stdlib.h>

#include "symboltable.h"

/**
 * Contains a collection of utility functions for the symbol
 * table data structure.
 */

/**
 * Creates a new symbol table node with the given symbol and address.
 * The pointer to the next node is initialized to null while every index
 * of the attributes array is marked as empty.
 * This function returns a pointer to the new symbol table node or a null
 * pointer if it failed to create it.
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
 * Returns a code to determine if the operation was successful or not.
 */
Code addAttribute(SymbolTable *symbolTable, unsigned char attributeCode) {
	int i;

	for (i = 0; i < ATTRS_PER_LABEL; ++i) {
		/* Checks if a new attribute code can be inserted. */
		if (symbolTable->attributes[i] == EMPTY) {
			/* Adding the attribute code, the operation was successful. */
			symbolTable->attributes[i] = attributeCode;
			return SUCCESS;
		}
	}
	/* The attribute code was not added, the operation failed. */
	return ERROR;
}

/**
 * Frees all the memory used by the symbol table data structure.
 */
void freeSymbolTable(SymbolTable *symbolTable) {
	SymbolTable *temp; /* Temporary pointer. */
	while (symbolTable != NULL) {
		/*
		Using the temporary pointer to free every node without loosing the
		next one.
		*/
		temp = symbolTable;
		symbolTable = symbolTable->next;
		free(temp);
	}
}
