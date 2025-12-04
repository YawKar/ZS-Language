#ifndef LEXICAL_ANALYSIS_H_
#define LEXICAL_ANALYSIS_H_

#include "Enums.h"
#include "Structs.h"

size_t CheckAndReturn(DifRoot *root, const char **string, Stack_Info *tokens);

#endif // LEXICAL_ANALYSIS_H_