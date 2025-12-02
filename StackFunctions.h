#ifndef STACK_FUNCTIONS_H_
#define STACK_FUNCTIONS_H_

#include <stdio.h>

#include "Enums.h"
#include "Structs.h"

DifErrors StackCtor(Stack_Info *stk, ssize_t capacity, FILE *open_log_file);
DifErrors StackPush(Stack_Info *stk, DifNode_t *value, FILE *open_log_file);
DifErrors StackPop(Stack_Info *stk, DifNode_t **value, FILE *open_log_file);
DifErrors StackRealloc(Stack_Info *stk, FILE *open_log_file, Realloc_Mode realloc_type);
DifErrors StackDtor(Stack_Info *stk, FILE *open_log_file);

#endif //STACK_FUNCTIONS_H_