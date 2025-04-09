#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "assembler.h"

/* Function prototypes */
bool openFiles(AssemblerContext* context, const char* filename);
bool createOutputFiles(AssemblerContext* context);
void closeFiles(AssemblerContext* context);
bool writeOutputFiles(AssemblerContext* context);
void wordToHex(Word word, char* hexStr);

#endif /* FILE_HANDLER_H */
