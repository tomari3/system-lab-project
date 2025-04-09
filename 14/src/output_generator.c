#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/output_generator.h"
#include "../include/constants.h"
#include "../include/data_structures.h"

/* --- Private Helper Function Declarations --- */
static FILE* open_output_file(const char *base_filename, const char *extension);
static int write_object_file(const char *base_filename, MachineWordNode *code_image, MachineWordNode *data_image, int icf, int dcf);
static int write_entries_file(const char *base_filename, SymbolNode *symbol_table);
static int write_externals_file(const char *base_filename, ExternUsageNode *extern_list);

/* --- Public Function Implementation --- */

/**
 * @brief Generates the final output files (.ob, .ent, .ext) for the assembler.
 */
int generate_output_files(const char *base_filename, SymbolNode *symbol_table,
                          MachineWordNode *code_image, MachineWordNode *data_image,
                          ExternUsageNode *extern_list, int icf, int dcf)
{
    int success = TRUE;

    printf("Generating output files...\n");

    if (!write_object_file(base_filename, code_image, data_image, icf, dcf)) {
        fprintf(stderr, "Error: Failed to write object file (.ob).\n");
        success = FALSE;
    } else {
        printf("Object file created: %s.ob\n", base_filename);
    }

    if (!write_entries_file(base_filename, symbol_table)) {
        /* Error message printed within helper */
        success = FALSE; /* Assume failure if helper returns false */
    }

    if (!write_externals_file(base_filename, extern_list)) {
        /* Error message printed within helper */
       success = FALSE; /* Assume failure if helper returns false */
    }

    return success;
}


/* --- Private Helper Function Implementations --- */

/** Opens an output file with the given base name and extension. */
static FILE* open_output_file(const char *base_filename, const char *extension) {
    char filename[FILENAME_MAX]; /* Use standard buffer size */
    FILE *fp = NULL;
    size_t base_len = strlen(base_filename);
    size_t ext_len = strlen(extension);

    /* Basic check for buffer overflow before sprintf */
    if (base_len + ext_len >= FILENAME_MAX) {
         fprintf(stderr, "Error: Output filename too long for '%s%s'.\n", base_filename, extension);
         return NULL;
    }

    sprintf(filename, "%s%s", base_filename, extension); /* Use sprintf */

    fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open output file '%s' for writing.\n", filename);
    }
    return fp;
}

/** Writes the object file (.ob). */
static int write_object_file(const char *base_filename, MachineWordNode *code_image, MachineWordNode *data_image, int icf, int dcf) {
    FILE *fp_ob = NULL;
    MachineWordNode *current = NULL;
    int code_len = icf - MEMORY_START_ADDRESS;
    int data_len = dcf;

    /* Handle potential negative length if icf is somehow less than start */
    if (code_len < 0) code_len = 0;

    fp_ob = open_output_file(base_filename, ".ob");
    if (!fp_ob) return FALSE;

    fprintf(fp_ob, "%d %d\n", code_len, data_len);

    current = code_image;
    while (current != NULL) {
        fprintf(fp_ob, "%07d %06x\n", current->address, current->word & 0xFFFFFF); /* Mask to 24 bits */
        current = current->next;
    }

    current = data_image;
    while (current != NULL) {
        fprintf(fp_ob, "%07d %06x\n", current->address, current->word & 0xFFFFFF); /* Mask to 24 bits */
        current = current->next;
    }

    fclose(fp_ob);
    return TRUE;
}

/** Writes the entries file (.ent). Only creates file if entries exist. */
static int write_entries_file(const char *base_filename, SymbolNode *symbol_table) {
    FILE *fp_ent = NULL;
    SymbolNode *current = symbol_table;
    int entries_found = FALSE;

    while (current != NULL) {
        if (current->is_entry) {
            entries_found = TRUE;
            break;
        }
        current = current->next;
    }

    if (!entries_found) return TRUE;

    fp_ent = open_output_file(base_filename, ".ent");
    if (!fp_ent) return FALSE;

    current = symbol_table;
    while (current != NULL) {
        if (current->is_entry) {
            fprintf(fp_ent, "%s %07d\n", current->name, current->value);
        }
        current = current->next;
    }

    fclose(fp_ent);
    printf("Entries file created: %s.ent\n", base_filename);
    return TRUE;
}

/** Writes the externals file (.ext). Only creates file if externals exist. */
static int write_externals_file(const char *base_filename, ExternUsageNode *extern_list) {
    FILE *fp_ext = NULL;
    ExternUsageNode *current = extern_list;

    if (extern_list == NULL) return TRUE;

    fp_ext = open_output_file(base_filename, ".ext");
    if (!fp_ext) return FALSE;

    current = extern_list;
    while (current != NULL) {
        fprintf(fp_ext, "%s %07d\n", current->symbol_name, current->usage_address);
        current = current->next;
    }

    fclose(fp_ext);
    printf("Externals file created: %s.ext\n", base_filename);
    return TRUE;
}

