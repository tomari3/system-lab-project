#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include "constants.h" /* For TRUE/FALSE */

/**
 * @brief Processes an assembly source file (.as) to handle macro definitions
 * and expansions, producing an intermediate file (.am).
 *
 * Reads the input file, identifies macro definitions, stores them,
 * and then reads the input file again (conceptually, or in one pass if possible)
 * to write an output file where macro definitions are removed and macro
 * invocations are replaced with their corresponding body lines.
 * Assumes macro definitions appear before their first use as per spec.
 *
 * @param input_filename The path to the input .as file.
 * @param output_filename The path where the output .am file should be created.
 * @return TRUE (1) if successful and no errors were found, FALSE (0) otherwise.
 * Errors are printed to stderr during processing.
 */
int run_pre_assembler(const char *input_filename, const char *output_filename);


#endif /* PRE_ASSEMBLER_H */

