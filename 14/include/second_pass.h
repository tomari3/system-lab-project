#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "data_structures.h" /* For SymbolNode, MachineWordNode, ExternUsageNode */
#include "constants.h"       /* For TRUE/FALSE */

/**
 * @brief Performs the second pass of the assembly process.
 *
 * Reads the intermediate file (.am), resolves symbol addresses using the
 * symbol table, generates the final machine code images (for instructions
 * and data), handles .entry directives, and builds a list of external
 * symbol usages.
 *
 * @param am_filename Path to the intermediate assembly file (.am).
 * @param symbol_table Head pointer of the symbol table (populated by first pass).
 * @param code_image_head Pointer to the head pointer of the code image linked list.
 * This will be populated by the function.
 * @param data_image_head Pointer to the head pointer of the data image linked list.
 * This will be populated by the function.
 * @param extern_list_head Pointer to the head pointer of the external symbol usage list.
 * This will be populated by the function.
 * @param icf The final Instruction Counter value from the first pass.
 * @param dcf The final Data Counter value from the first pass.
 * @return TRUE (1) if the second pass completed without errors, FALSE (0) otherwise.
 * Errors encountered are printed to stderr. The caller is responsible for freeing
 * the allocated lists (*code_image_head, *data_image_head, *extern_list_head)
 * using functions like free_machine_word_list and free_extern_list.
 */
int run_second_pass(const char *am_filename, SymbolNode *symbol_table,
                    MachineWordNode **code_image_head, MachineWordNode **data_image_head,
                    ExternUsageNode **extern_list_head, int icf, int dcf);


/**
 * @brief Frees the memory allocated for a machine word linked list (code or data image).
 * @param head The head pointer of the list to free.
 */
void free_machine_word_list(MachineWordNode *head);

/**
 * @brief Frees the memory allocated for an external symbol usage linked list.
 * @param head The head pointer of the list to free.
 */
void free_extern_list(ExternUsageNode *head);


#endif /* SECOND_PASS_H */

