#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "data_structures.h" /* For SymbolNode, SymbolAttribute */
#include "constants.h"       /* For TRUE/FALSE */

/**
 * @brief Adds a symbol to the symbol table (linked list).
 * Checks for redefinition errors. Allocates memory for the new node and name.
 *
 * @param head Pointer to the head pointer of the symbol table list.
 * @param name The name of the symbol (label).
 * @param value The address/value of the symbol.
 * @param type The base type (ATTR_CODE, ATTR_DATA, ATTR_EXTERNAL).
 * @param line_num The line number where the symbol is defined (for error messages).
 * @param error_flag Pointer to an integer flag, set to TRUE if an error occurs.
 * @return Pointer to the newly added symbol node, or NULL if allocation failed
 * or redefinition occurred (error_flag will be set).
 */
SymbolNode* add_symbol(SymbolNode **head, const char *name, int value, SymbolAttribute type, int line_num, int *error_flag);

/**
 * @brief Finds a symbol by name in the symbol table.
 *
 * @param head The head pointer of the symbol table list.
 * @param name The name of the symbol to find.
 * @return Pointer to the found SymbolNode, or NULL if not found.
 */
SymbolNode* find_symbol(SymbolNode *head, const char *name);

/**
 * @brief Marks an existing symbol as also being an entry point.
 * Handles cases where the symbol doesn't exist or is external.
 *
 * @param head The head pointer of the symbol table list.
 * @param name The name of the symbol to mark.
 * @param line_num The line number where the .entry directive occurred (for errors).
 * @param error_flag Pointer to an integer flag, set to TRUE if an error occurs.
 */
void mark_symbol_as_entry(SymbolNode *head, const char *name, int line_num, int *error_flag);


/**
 * @brief Relocates (updates) the addresses of all data symbols by adding the
 * final instruction counter value (ICF).
 *
 * @param head The head pointer of the symbol table list.
 * @param icf_value The final value of the instruction counter (size of code section).
 */
void relocate_data_symbols(SymbolNode *head, int icf_value);


/**
 * @brief Frees all memory associated with the symbol table linked list.
 *
 * @param head The head pointer of the symbol table list.
 */
void free_symbol_table(SymbolNode *head);


#endif /* SYMBOL_TABLE_H */

