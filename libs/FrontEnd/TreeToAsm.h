#ifndef TREE_TO_ASM_H_
#define TREE_TO_ASM_H_

#include "Common/Structs.h"

void PrintProgram(
    FILE* file, LangNode_t* root, VariableArr* arr, int* ram_base
);

void TreeToAsmReinitGlobals();

#endif  // TREE_TO_ASM_H_
