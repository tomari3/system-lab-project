#include "../include/assembler.h"
#include "../include/preprocessor.h"
#include "../include/first_pass.h"
#include "../include/second_pass.h"
#include "../include/file_handler.h"
#include "../include/error_handler.h"

/* Create a new assembler context */
AssemblerContext* createContext() {
    AssemblerContext* context = (AssemblerContext*)malloc(sizeof(AssemblerContext));
    if (!context) {
        return NULL;
    }
    
    /* Initialize the context */
    context->IC = MEMORY_START;
    context->DC = 0;
    context->lineNumber = 0;
    context->symbolTable = NULL;
    context->macroTable = NULL;
    context->errorList = NULL;
    context->codeImage = NULL;
    context->dataImage = NULL;
    context->codeImageSize = 0;
    context->dataImageSize = 0;
    context->hasErrors = false;
    context->sourceFile = NULL;
    context->amFile = NULL;
    context->objectFile = NULL;
    context->externalsFile = NULL;
    context->entriesFile = NULL;
    context->sourceFileName[0] = '\0';
    
    return context;
}

/* Free all resources in the assembler context */
void freeContext(AssemblerContext* context) {
    if (!context) {
        return;
    }
    
    /* Close all open files */
    closeFiles(context);
    
    /* Free the symbol table */
    freeSymbolTable(context->symbolTable);
    
    /* Free the macro table */
    freeMacroTable(context->macroTable);
    
    /* Free the error list */
    freeErrorList(context->errorList);
    
    /* Free the code and data images */
    if (context->codeImage) {
        free(context->codeImage);
    }
    
    if (context->dataImage) {
        free(context->dataImage);
    }
    
    /* Free the context itself */
    free(context);
}

/* Main assembly function for a single file */
bool assembleFile(const char* filename) {
    AssemblerContext* context;
    bool success = true;
    
    /* Create the assembler context */
    context = createContext();
    if (!context) {
        fprintf(stderr, "Error: Failed to initialize assembler\n");
        return false;
    }
    
    /* Open the source file */
    if (!openFiles(context, filename)) {
        fprintf(stderr, "Error: Failed to open files for '%s'\n", filename);
        freeContext(context);
        return false;
    }
    
    printf("Assembling file: %s.as\n", filename);
    
    /* Step 1: Preassembler (macro expansion) */
    printf("Pass 0: Preprocessing (expanding macros)...\n");
    if (!preprocess(context)) {
        printf("Preprocessing failed.\n");
        printErrors(context);
        freeContext(context);
        return false;
    }
    
    /* Step 2: First pass */
    printf("Pass 1: Building symbol table and initial encoding...\n");
    if (!firstPass(context)) {
        printf("First pass failed.\n");
        printErrors(context);
        freeContext(context);
        return false;
    }
    
    /* Step 3: Second pass */
    printf("Pass 2: Completing the encoding process...\n");
    if (!secondPass(context)) {
        printf("Second pass failed.\n");
        printErrors(context);
        freeContext(context);
        return false;
    }
    
    /* Step 4: Write output files */
    printf("Writing output files...\n");
    if (!writeOutputFiles(context)) {
        printf("Failed to write output files.\n");
        freeContext(context);
        return false;
    }
    
    printf("Assembly completed successfully.\n");
    printf("Generated files:\n");
    printf("  %s.am (expanded source)\n", filename);
    printf("  %s.ob (object file)\n", filename);
    
    /* Check if entries file was created */
    if (context->entriesFile) {
        printf("  %s.ent (entries file)\n", filename);
    }
    
    /* Check if externals file was created */
    if (context->externalsFile) {
        printf("  %s.ext (externals file)\n", filename);
    }
    
    /* Free all resources */
    freeContext(context);
    
    return success;
}
