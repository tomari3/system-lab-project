#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "data_structures.h" /* For SymbolNode */
#include "constants.h"       /* For TRUE/FALSE */

/**
 * @brief Performs the first pass of the assembly process.
 *
 * Reads the intermediate file (.am), builds the symbol table, calculates
 * initial instruction (IC) and data (DC) counter values, and detects syntax
 * and symbol errors. Relocates data symbols at the end.
 *
 * @param am_filename Path to the intermediate assembly file (.am).
 * @param symbol_table_head Pointer to the head pointer of the symbol table.
 * This will be populated by the function.
 * @param icf Pointer to an integer where the final Instruction Counter (ICF)
 * value will be stored.
 * @param dcf Pointer to an integer where the final Data Counter (DCF)
 * value will be stored.
 * @return TRUE (1) if the first pass completed without errors, FALSE (0) otherwise.
 * Errors encountered are printed to stderr.
 */
int run_first_pass(const char *am_filename, SymbolNode **symbol_table_head, int *icf, int *dcf);


#endif /* FIRST_PASS_H */

