#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/pre_assembler.h" 

/* Placeholder for future function declarations */
/* e.g., void first_pass(const char *am_filename); */

/**
 * @brief Main entry point for the assembler.
 *
 * @param argc Argument count.
 * @param argv Argument vector. Expects source filenames without extension.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
    int i;
    int overall_success = TRUE; /* Track if all files processed successfully */

    /* Check if any filenames were provided */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2] ...\n", argv[0]);
        fprintf(stderr, "(Provide filenames without the .as extension)\n");
        return EXIT_FAILURE;
    }

    printf("--- Assembler Started ---\n");

    /* Loop through each filename provided */
    for (i = 1; i < argc; ++i) {
        char input_filename[MAX_LINE_LENGTH];
        char am_filename[MAX_LINE_LENGTH];
        int pre_assembler_ok;

        printf("\nProcessing file: %s\n", argv[i]);

        /* Construct filenames */
        snprintf(input_filename, sizeof(input_filename), "%s.as", argv[i]);
        snprintf(am_filename, sizeof(am_filename), "%s.am", argv[i]); /* Output to current dir for simplicity */
        /* Consider putting .am files in build dir later */

        /* --- Stage 1: Pre-Assembler --- */
        printf("Running pre-assembler...\n");
        pre_assembler_ok = run_pre_assembler(input_filename, am_filename);

        if (!pre_assembler_ok) {
            fprintf(stderr, "Pre-assembler failed for %s. Skipping further processing.\n", input_filename);
            overall_success = FALSE;
            continue; /* Move to next file */
        }
        printf("Pre-assembler successful. Output: %s\n", am_filename);


        /* --- Stage 2: First Pass (Placeholder) --- */
        printf("Placeholder: First pass would run on %s\n", am_filename);
        /*
         * TODO: Call first_pass(am_filename) -> generates symbol table, partial code, checks errors
         * if (!first_pass_ok) { overall_success = FALSE; continue; }
         */

        /* --- Stage 3: Second Pass (Placeholder) --- */
        printf("Placeholder: Second pass would run\n");
         /*
         * TODO: Call second_pass(am_filename, symbol_table, ...) -> completes code, checks errors
         * if (!second_pass_ok) { overall_success = FALSE; continue; }
         */

        /* --- Stage 4: Output Generation (Placeholder) --- */
         printf("Placeholder: Output file generation would happen\n");
        /*
         * TODO: Call generate_output_files(...)
         */

        printf("Successfully processed argument: %s\n", argv[i]);

    } /* End loop through files */

    printf("\n--- Assembler Finished ---\n");

    return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
