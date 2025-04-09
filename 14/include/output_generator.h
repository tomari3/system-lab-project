#ifndef OUTPUT_GENERATOR_H
#define OUTPUT_GENERATOR_H

#include "data_structures.h" /* For SymbolNode, MachineWordNode, ExternUsageNode */
#include "constants.h"       /* For TRUE/FALSE */

/**
 * @brief Generates the final output files (.ob, .ent, .ext) for the assembler.
 *
 * Creates the object file containing the machine code, the entry points file
 * (if any entry points exist), and the external references file (if any
 * external symbols were used).
 *
 * @param base_filename The base name of the input file (without extension),
 * used to construct output filenames.
 * @param symbol_table Head pointer of the symbol table.
 * @param code_image Head pointer of the code image linked list.
 * @param data_image Head pointer of the data image linked list.
 * @param extern_list Head pointer of the external symbol usage list.
 * @param icf The final Instruction Counter value (address after last instruction).
 * @param dcf The final Data Counter value (total number of data words).
 * @return TRUE (1) if all necessary files were generated successfully, FALSE (0)
 * if any file operation failed. Errors are printed to stderr.
 */
int generate_output_files(const char *base_filename, SymbolNode *symbol_table,
                          MachineWordNode *code_image, MachineWordNode *data_image,
                          ExternUsageNode *extern_list, int icf, int dcf);


#endif /* OUTPUT_GENERATOR_H */

