#include <stdlib.h>
#include <string.h>

#include "symboltable.h"
#include "asmutils.h"

/**
 * Contains a collection of utility functions for the symbol
 * table data structure.
 */

#define ATTRS_PER_LABEL 2 /* Maximum number of attributes per label. */

/**
 * The following function should not be used outside of this translation unit.
 */
SymbolTable *createSymbol(char *symbol, unsigned address);

/**
 * Defining the symbol table data structure.
 * This structure is used to map all labels in the assembly code,
 * with every label having its assigned address and attributes.
 * A pointer to this structure points to a single label.
 */
struct symbolt {
	char *symbol; /* Stores the symbol of that label. */
	unsigned long int address; /* Stores the memory address of that label. */
	unsigned char attributes[ATTRS_PER_LABEL]; /* Stores the attributes codes of that label. */
	struct symbolt *next; /* A pointer to the next label. */
};

/**
 * Returns the address of the given label.
 */
unsigned long int getAddress(SymbolTable *symbolTable) {
	return symbolTable->address;
}

/**
 * Returns the string representation of the given label.
 */
char *getSymbol(SymbolTable *symbolTable) {
	return symbolTable->symbol;
}

/**
 * Returns a pointer to the next label after the given label.
 * The labels are sorted the way they were added.
 */
SymbolTable *getNext(SymbolTable *symbolTable) {
	return symbolTable->next;
}

/**
 * Searches through the given symbol table for the label that
 * has the same symbol as the given string.
 * Returns a pointer to that label if there is one with a
 * matching symbol, null if otherwise.
 */
SymbolTable *searchLabel(SymbolTable *symbolTable, char *symbol) {
	/* Looping through the elements in the data structure. */
	while (symbolTable != NULL) {
		if (strcmp(symbolTable->symbol, symbol) == 0)
			/* If a label with an identical symbol was found a pointer to it would be returned. */
			return symbolTable;
		symbolTable = symbolTable->next; /* Continuing to the next element. */
	}
	return NULL; /* No matching label was found, a null pointer is returned. */
}

/**
 * Checks if the given label has the given attribute, if it does then
 * a SUCCESS code would be returned. If the given label does not have
 * the given attribute assign to it then ERROR would be returned
 * instead.
 */
Code hasAttribute(SymbolTable *symbolTable, LabelAttribute labelAttribute) {
	int index;

	/* Searching trough the attributes array of the given label. */
	for (index = 0; index < ATTRS_PER_LABEL; index++)
		if (symbolTable->attributes[index] == labelAttribute)
			/* A matching attribute was found. */
			return SUCCESS;
	/* No matching attribute was found. */
	return ERROR;
}

/**
 * Adds a new label to the given symbol table and assigns it the given
 * symbol and the given address.
 * In case the given symbol table is null the newly created label can
 * still be accessed via the returned pointer.
 * Returns a pointer to the newly created label if the label was
 * created successfully, and a null pointer if otherwise.
 */
SymbolTable *addSymbol(SymbolTable *symbolTable, char *symbol, unsigned address) {
	/* Storing the newly created label. */
	SymbolTable *newLabel = createSymbol(symbol, address);

	/* If the given label is a null pointer then there is nothing to skip to. */
	if (symbolTable != NULL) {
		/* Skipping to the end of the list. */
		while (symbolTable->next != NULL)
			symbolTable = symbolTable->next;

		/*
		 * Adding the new label with the given parameters to the end of the
		 * given list.
		 */
		symbolTable->next = newLabel;
	}

	/* Returning a pointer to the new label. */
	return newLabel;
}

/**
 * Creates a new symbol table node with the given symbol and address.
 * The pointer to the next node is initialized to null while every index
 * of the attributes array is marked as empty.
 * This function returns a pointer to the new symbol table node or a null
 * pointer if it failed to create it.
 * Used internally by the addSymbol function.
 */
SymbolTable *createSymbol(char *symbol, unsigned address) {
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
	for (i = 0; i < ATTRS_PER_LABEL; i++) {
		symbolTable->attributes[i] = EmptyLabel;
	}

	return symbolTable; /* Returning the address of the new symbol table node. */
}

/**
 * Adds the given attribute code to the given label.
 * Returns a code to determine if the operation was successful or not.
 */
Code addAttribute(SymbolTable *symbolTable, LabelAttribute labelAttribute) {
	int index;

	for (index = 0; index < ATTRS_PER_LABEL; index++) {
		/* Checks if a new attribute code can be inserted. */
		if (symbolTable->attributes[index] == EmptyLabel) {
			/* Adding the attribute code, the operation was successful. */
			symbolTable->attributes[index] = labelAttribute;
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
		free(temp->symbol);
		free(temp);
	}
}
