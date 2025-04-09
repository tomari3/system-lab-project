#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/pre_assembler.h"
#include "../include/first_pass.h"
#include "../include/second_pass.h"   
#include "../include/output_generator.h" 
#include "../include/symbol_table.h"
#include "../include/data_structures.h"

/**
 * @brief Main entry point for the assembler.
 */
int main(int argc, char *argv[]) {
    int i;
    int overall_success = TRUE;
    const char *as_ext = ".as";
    const char *am_ext = ".am";
    size_t as_ext_len = strlen(as_ext);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1[.as]> [file2[.as]] ...\n", argv[0]);
        fprintf(stderr, "(Provide filenames relative to the current directory, .as extension is optional)\n");
        return EXIT_FAILURE;
    }

    printf("--- Assembler Started ---\n");

    for (i = 1; i < argc; ++i) {
        char input_filename[MAX_LINE_LENGTH * 2];
        char am_filename[MAX_LINE_LENGTH * 2];
        char base_name[MAX_LINE_LENGTH];
        char output_base_name[MAX_LINE_LENGTH * 2]; /* For path + base name */
        char *last_slash = NULL;
        char *dot_pos = NULL;
        int pre_assembler_ok;
        int first_pass_ok;
        int second_pass_ok;
        int output_ok; /* Added flag */
        size_t arg_len = strlen(argv[i]);
        SymbolNode *symbol_table = NULL;
        int icf = 0;
        int dcf = 0;
        MachineWordNode *code_image = NULL;
        MachineWordNode *data_image = NULL;
        ExternUsageNode *extern_list = NULL;

        printf("\nProcessing file argument: %s\n", argv[i]);

        /* --- Construct filenames --- */
        if (arg_len > as_ext_len && strcmp(argv[i] + arg_len - as_ext_len, as_ext) == 0) {
            strncpy(input_filename, argv[i], sizeof(input_filename) - 1);
        } else {
            snprintf(input_filename, sizeof(input_filename), "%s%s", argv[i], as_ext);
        }
        input_filename[sizeof(input_filename) - 1] = '\0';

        last_slash = strrchr(argv[i], '/');
        if (last_slash) { strncpy(base_name, last_slash + 1, sizeof(base_name) - 1); }
        else { strncpy(base_name, argv[i], sizeof(base_name) - 1); }
        base_name[sizeof(base_name) - 1] = '\0';
        dot_pos = strrchr(base_name, '.');
        if (dot_pos && strcmp(dot_pos, as_ext) == 0) { *dot_pos = '\0'; }
        snprintf(am_filename, sizeof(am_filename), "build/%s%s", base_name, am_ext);
        /* Base name for output files (without path initially) */
        strncpy(output_base_name, base_name, sizeof(output_base_name) -1);
        output_base_name[sizeof(output_base_name) - 1] = '\0';


        /* --- Stage 1: Pre-Assembler --- */
        printf("Running pre-assembler...\nInput: %s\nOutput: %s\n", input_filename, am_filename);
        pre_assembler_ok = run_pre_assembler(input_filename, am_filename);
        if (!pre_assembler_ok) {
            fprintf(stderr, "Pre-assembler failed for %s. Skipping.\n", input_filename);
            overall_success = FALSE; continue;
        }
        printf("Pre-assembler successful.\n");

        /* --- Stage 2: First Pass --- */
        printf("Running first pass...\n");
        first_pass_ok = run_first_pass(am_filename, &symbol_table, &icf, &dcf);
        if (!first_pass_ok) {
             fprintf(stderr, "First pass failed for %s. Skipping.\n", am_filename);
             overall_success = FALSE; symbol_table = NULL; continue;
        }

        /* --- Stage 3: Second Pass --- */
        printf("Running second pass...\n");
        second_pass_ok = run_second_pass(am_filename, symbol_table, &code_image, &data_image, &extern_list, icf, dcf);
        if (!second_pass_ok) {
             fprintf(stderr, "Second pass failed for %s. Skipping output generation.\n", am_filename);
             overall_success = FALSE;
             free_machine_word_list(code_image); code_image = NULL;
             free_machine_word_list(data_image); data_image = NULL;
             free_extern_list(extern_list); extern_list = NULL;
             free_symbol_table(symbol_table); symbol_table = NULL;
             continue;
        }
        printf("Second pass successful.\n");

        /* --- Stage 4: Output Generation --- */
        output_ok = generate_output_files(output_base_name, symbol_table, code_image, data_image, extern_list, icf, dcf);
        if (!output_ok) {
             fprintf(stderr, "Failed to generate output files for %s.\n", output_base_name);
             overall_success = FALSE;
             /* Still need to cleanup lists even if output failed */
        }

        printf("Successfully processed argument: %s\n", argv[i]);

        /* --- Cleanup for this file --- */
        free_machine_word_list(code_image); code_image = NULL;
        free_machine_word_list(data_image); data_image = NULL;
        free_extern_list(extern_list); extern_list = NULL;
        free_symbol_table(symbol_table); symbol_table = NULL;

    } /* End loop through files */

    printf("\n--- Assembler Finished ---\n");
    return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}

