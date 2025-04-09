#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/output_generator.h"
#include "../include/constants.h"
#include "../include/data_structures.h"

/* --- Private Helper Function Declarations --- */

/** Opens an output file with the given base name and extension. Returns NULL on error. */
static FILE* open_output_file(const char *base_filename, const char *extension);
/** Writes the object file (.ob). */
static int write_object_file(const char *base_filename, MachineWordNode *code_image, MachineWordNode *data_image, int icf, int dcf);
/** Writes the entries file (.ent). Only creates file if entries exist. */
static int write_entries_file(const char *base_filename, SymbolNode *symbol_table);
/** Writes the externals file (.ext). Only creates file if externals exist. */
static int write_externals_file(const char *base_filename, ExternUsageNode *extern_list);


/* --- Public Function Implementation --- */

int generate_output_files(const char *base_filename, SymbolNode *symbol_table,
                          MachineWordNode *code_image, MachineWordNode *data_image,
                          ExternUsageNode *extern_list, int icf, int dcf)
{
    int success = TRUE;

    printf("Generating output files...\n");

    /* Write .ob file */
    if (!write_object_file(base_filename, code_image, data_image, icf, dcf)) {
        fprintf(stderr, "Error: Failed to write object file (.ob).\n");
        success = FALSE;
    } else {
        printf("Object file created: %s.ob\n", base_filename);
    }

    /* Write .ent file (conditionally) */
    if (!write_entries_file(base_filename, symbol_table)) {
        /* Error message printed within the function if file needed but failed */
        /* If no entries, function returns TRUE without creating file */
        if (success) { /* Only flag failure if not already failed */
             /* Check if an actual error occurred vs just no entries */
             /* This check might need refinement depending on how write_entries_file signals errors */
        }
    }

    /* Write .ext file (conditionally) */
    if (!write_externals_file(base_filename, extern_list)) {
         /* Error message printed within the function if file needed but failed */
         /* If no externals, function returns TRUE without creating file */
        if (success) { /* Only flag failure if not already failed */
             /* Check if an actual error occurred vs just no externals */
        }
    }

    return success;
}


/* --- Private Helper Function Implementations --- */

/** Opens an output file with the given base name and extension. */
static FILE* open_output_file(const char *base_filename, const char *extension) {
    char filename[MAX_LINE_LENGTH * 2]; /* Allow space for path/name + extension */
    FILE *fp = NULL;

    /* Construct filename (assuming base_filename doesn't contain path for now) */
    /* A more robust solution would handle paths properly */
    sprintf(filename, "%s%s", base_filename, extension); /* Use sprintf (or snprintf if preferred) */

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

    fp_ob = open_output_file(base_filename, ".ob");
    if (!fp_ob) {
        return FALSE;
    }

    /* Write header: Code Length, Data Length */
    fprintf(fp_ob, "%d %d\n", code_len, data_len);

    /* Write code image */
    current = code_image;
    while (current != NULL) {
        fprintf(fp_ob, "%07d %06x\n", current->address, current->word);
        current = current->next;
    }

    /* Write data image */
    current = data_image;
    while (current != NULL) {
        fprintf(fp_ob, "%07d %06x\n", current->address, current->word);
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

    /* First pass: check if any entries exist */
    while (current != NULL) {
        if (current->is_entry) {
            entries_found = TRUE;
            break;
        }
        current = current->next;
    }

    /* Only create file if entries were found */
    if (!entries_found) {
        return TRUE; /* Not an error, just nothing to write */
    }

    /* Open file */
    fp_ent = open_output_file(base_filename, ".ent");
    if (!fp_ent) {
        return FALSE; /* File opening failed */
    }

    /* Second pass: write entries */
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

    /* Only create file if list is not empty */
    if (extern_list == NULL) {
        return TRUE; /* Not an error, just nothing to write */
    }

    /* Open file */
    fp_ext = open_output_file(base_filename, ".ext");
    if (!fp_ext) {
        return FALSE; /* File opening failed */
    }

    /* Write external usages */
    current = extern_list;
    while (current != NULL) {
        fprintf(fp_ext, "%s %07d\n", current->symbol_name, current->usage_address);
        current = current->next;
    }

    fclose(fp_ext);
    printf("Externals file created: %s.ext\n", base_filename);
    return TRUE;
}

