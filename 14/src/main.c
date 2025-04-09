#include "../include/assembler.h"

int main(int argc, char* argv[]) {
    int i;
    bool success = true;
    
    /* Check for command-line arguments */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
        fprintf(stderr, "Note: Do not include file extensions.\n");
        return 1;
    }
    
    /* Process each input file */
    for (i = 1; i < argc; i++) {
        if (!assembleFile(argv[i])) {
            success = false;
        }
        
        /* Add a blank line between files */
        if (i < argc - 1) {
            printf("\n");
        }
    }
    
    return success ? 0 : 1;
}
