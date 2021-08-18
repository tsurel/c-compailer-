#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "asmutils.h"

/**
 * An header file for the symbol table translation unit.
 */

/**
 * Defining the symbol table data structure.
 * This structure is used to map all labels in the assembly code,
 * with every label having its assigned address and attributes.
 * A pointer to this structure points to a single label.
 */
typedef struct symbolt SymbolTable;

/**
 * Attributes for labels.
 * used for distinguishing different types of labels.
 */
typedef enum {
    EmptyLabel, /* No attribute mark. */
    CodeLabel, /* Code attribute. */
    DataLabel, /* Data attribute. */
    EntryLabel, /* Entry attribute. */
    ExternLabel /* External attribute. */
} LabelAttribute;

/**
 * Returns the address of the given label.
 */
unsigned long int getAddress(SymbolTable *symbolTable);

/**
 * Returns the string representation of the given label.
 */
char *getSymbol(SymbolTable *symbolTable);

/**
 * Sets the address field of the given label to the
 * given second parameter.
 */
void setAddress(SymbolTable *symbolTable, unsigned long int address);

/**
 * Returns a pointer to the next label after the given label.
 * The labels are sorted the way they were added.
 */
SymbolTable *getNext(SymbolTable *symbolTable);

/**
 * Searches through the given symbol table for the label that
 * has the same symbol as the given string.
 * Returns a pointer to that label if there is one with a
 * matching symbol, null if otherwise.
 */
SymbolTable *searchLabel(SymbolTable *symbolTable, const char *symbol);

/**
 * Checks if the given label has the given attribute, if it does then
 * a SUCCESS code would be returned. If the given label does not have
 * the given attribute assign to it then ERROR would be returned
 * instead.
 */
Code hasAttribute(SymbolTable *symbolTable, LabelAttribute labelAttribute);

/**
 * Checks if the given label has code or data attribute, if it does then a
 * SUCCESS code would be returned and ERROR if otherwise. Used
 * for checking if a label is declared.
 */
Code isDeclared(SymbolTable *symbolTable);

/**
 * Adds a new label to the given symbol table and assigns it the given
 * symbol and the given address.
 * In case the given symbol table is null the newly created label can
 * still be accessed via the returned pointer.
 * Returns a pointer to the newly created label if the label was
 * created successfully, and a null pointer if otherwise.
 */
SymbolTable *addSymbol(SymbolTable *symbolTable, char *symbol, unsigned long int address);

/**
 * Removes the given label on the second parameter from the given
 * symbol table on the first parameter.
 */
void removeSymbol(SymbolTable **symbolTable, SymbolTable *label);

/**
 * Adds the given attribute code to the given label.
 * Returns a code to determine if the operation was successful or not.
 */
Code addAttribute(SymbolTable *symbolTable , LabelAttribute labelAttribute);

/**
 * Frees all the memory used by the symbol table data structure.
 */
void freeSymbolTable(SymbolTable *symbolTable);

#endif
