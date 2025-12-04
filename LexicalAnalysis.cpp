#include "LexicalAnalysis.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Enums.h"
#include "Structs.h"
#include "StackFunctions.h"
#include "LanguageFunctions.h"

#define IF "if"
#define ELSE "else"
#define WHILE "while"

#define NEWN(num) NewNode(root, kNumber, ((Value){ .number = (num)}), NULL, NULL, NULL)
#define NEWOP(op, left, right) NewNode(root, kOperation, (Value){ .operation = (op)}, left, right, NULL) 

size_t CheckAndReturn(DifRoot *root, const char **string, Stack_Info *tokens) {
    assert(root);
    assert(string);
    assert(tokens);

    size_t cnt = 0;

    while (**string != '\0') {
        if (**string == '(') {
            StackPush(tokens, NEWOP(kOperationParOpen, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == ')') {
            StackPush(tokens, NEWOP(kOperationParClose, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '{') {
            StackPush(tokens, NEWOP(kOperationBraceOpen, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '}') {
            StackPush(tokens, NEWOP(kOperationBraceClose, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '=') {
            StackPush(tokens, NEWOP(kOperationIs, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == ';') {
            StackPush(tokens, NEWOP(kOperationThen, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '+') {
            StackPush(tokens, NEWOP(kOperationAdd, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '-') {
            StackPush(tokens, NEWOP(kOperationSub, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '*') {
            StackPush(tokens, NEWOP(kOperationMul, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '/') {
            StackPush(tokens, NEWOP(kOperationDiv, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }
        if (**string == '^') {
            StackPush(tokens, NEWOP(kOperationPow, NULL, NULL), stderr);
            cnt++;
            (*string)++;
            continue;
        }

        if (strncmp(*string, "sin", sizeof("sin")) == 0) {
            StackPush(tokens, NEWOP(kOperationSin, NULL, NULL), stderr);
            cnt++;
            (*string) += sizeof("sin");
            continue;
        }
        if (strncmp(*string, IF, sizeof(IF)) == 0) {
            StackPush(tokens, NEWOP(kOperationIf, NULL, NULL), stderr);
            cnt++;
            (*string) += sizeof(IF);
            continue;
        }
        if (strncmp(*string, WHILE, sizeof(WHILE)) == 0) {
            StackPush(tokens, NEWOP(kOperationWhile, NULL, NULL), stderr);
            cnt++;
            (*string) += sizeof(WHILE);
            continue;
        }
        if (strncmp(*string, ELSE, sizeof(ELSE)) == 0) {
            StackPush(tokens, NEWOP(kOperationElse, NULL, NULL), stderr);
            cnt++;
            (*string) += sizeof(ELSE);
            continue;
        }

        if ('0' <= **string && **string <= '9') {
            int value = 0;
            do {
                value = 10 * value + (**string - '0');
                (*string)++;
            } while ('0' <= **string && **string <= '9');
            StackPush(tokens, NEWN(value), stderr);
            cnt++;
            continue;
        }

        if (isalnum(**string) || **string == '_') {
            const char *str = *string;
            size_t str_size = 0;
            while (isalnum(**string) || **string == '_') {
                (*string)++;
                str_size++;
            }
            cnt++;
            StackPush(tokens, NewNode(root, kVariable, (Value){ .variable.variable_name = strdup(*string)}, NULL, NULL, NULL), stderr); //
            continue;
        }

        if (isspace(**string)) {
            while (isspace(**string)) {
                (*string)++;
                continue;
            }
        }

        fprintf(stderr, "AAAAAA, SYNTAX_ERROR.");
        return 0;
    }
    return cnt;
}