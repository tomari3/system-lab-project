#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "assembler.h"

/* Function prototypes */
bool firstPass(AssemblerContext* context);

/* Symbol table update function */
void updateDataSymbols(AssemblerContext* context, int icf);

#endif /* FIRST_PASS_H */
