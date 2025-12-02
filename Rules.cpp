#include "Rules.h"

#include "Enums.h"
#include "Structs.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "Enums.h"
#include "Structs.h"
#include "LanguageFunctions.h"
#include "DoGraph.h"

static long long SizeOfFile(const char *filename);
static char *ReadToBuf(const char *filename, FILE *file, size_t filesize);
static void DoBufRead(FILE *file, const char *filename, FileInfo *Info);

#define CHECK_NULL_RETURN(name, cond) \
    DifNode_t *name = cond;           \
    if (name == NULL) {               \
        return NULL;                  \
    }

static OpEntry operations[] = {
    {"tg",    kOperationTg},
    {"sin",   kOperationSin},
    {"cos",   kOperationCos},
    {"ln",    kOperationLn},
    {"arctg", kOperationArctg},
    {"pow",   kOperationPow},
    {"sh",    kOperationSinh},
    {"ch",    kOperationCosh},
    {"th",    kOperationTgh},
};
size_t max_op_size = 5; //

DifNode_t *GetGoal(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetExpression(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetTerm(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetPrimary(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetPower(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetAssignment(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetOp(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);

static DifNode_t *GetNumber(DifRoot *root, const char **string);
static DifNode_t *GetString(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetOperation(DifRoot *root, const char **string, VariableArr *arr, size_t *position, char *buffer);
static OperationTypes ParseOperator(const char *string);

DifErrors ReadInfix(DifRoot *root, DumpInfo *dump_info, VariableArr *Variable_Array, const char *filename, FILE *texfile) {
    assert(root);
    assert(dump_info);
    assert(Variable_Array);
    assert(filename);
    assert(texfile);

    FILE_OPEN_AND_CHECK(file, filename, "r");

    FileInfo Info = {};
    DoBufRead(file, filename, &Info);
    printf("%s", Info.buf_ptr);

    fclose(file);

    size_t pos = 0;
    const char *new_string = Info.buf_ptr;
    root->root = GetGoal(root, &new_string, Variable_Array, &pos);
    if (!root->root) {
        return kFailure;
    }
    
    dump_info->tree = root;

    strcpy(dump_info->message, "Expression read with infix form");
    DoTreeInGraphviz(root->root, dump_info, root->root);

    //PrintFirstExpression(texfile, root->root);
    //DoDump(dump_info);

    return kSuccess;
}

#define NEWN(num) NewNode(root, kNumber, ((Value){ .number = (num)}), NULL, NULL, arr)
#define NEWOP(op, left, right) NewNode(root, kOperation, (Value){ .operation = (op) }, left, right, arr) 

DifNode_t *GetGoal(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);
    
    DifNode_t *first = GetOp(root, string, arr, pos);
    if (!first) {
        fprintf(stderr, "SYNTAX ERROR: expected statement\n");
        return NULL;
    }

    while (**string == ';') {
        (*string)++;

        DifNode_t *next = GetOp(root, string, arr, pos);
        if (!next) {
            fprintf(stderr, "SYNTAX ERROR: expected statement after ';'\n");
            return NULL;
        }

        first = NewNode(root, kOperation, (Value){ .operation = kOperationThen }, first, next, arr);
    }

    if (**string == '$') {
        (*string)++;
        return first;
    }

    fprintf(stderr, "SYNTAX ERROR: expected ';' or '$', got '%c'\n", **string);
    return NULL;
}


#define NEWN(num) NewNode(root, kNumber, ((Value){ .number = (num)}), NULL, NULL, arr)
#define ADD_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationAdd}, left, right, arr)
#define SUB_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationSub}, left, right, arr)
#define MUL_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationMul}, left, right, arr)
#define DIV_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationDiv}, left, right, arr)
#define POW_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationPow}, left, right, arr)

static DifNode_t *GetExpression(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    CHECK_NULL_RETURN(val, GetTerm(root, string, arr, pos));
    root->size++;

    while (**string == '+' || **string == '-') {
        int op = **string;
        (*string)++;
        CHECK_NULL_RETURN(val2, GetTerm(root, string, arr, pos));

        if (op == '+') {
            val = ADD_(val, val2);
        } else if (op == '-') {
            val = SUB_(val, val2);
        }
        root->size += 1;
    }

    return val;
}

static DifNode_t *GetTerm(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    CHECK_NULL_RETURN(val, GetPower(root, string, arr, pos));

    while (**string == '*' || **string == '/') {
        char op = **string;
        (*string)++;

        CHECK_NULL_RETURN(val2, GetPower(root, string, arr, pos));

        if (op == '*')
            val = MUL_(val, val2);
        else
            val = DIV_(val, val2);
    }

    return val;
}

static DifNode_t *GetPrimary(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    if (**string == '(') {
        (*string)++;
        CHECK_NULL_RETURN(val, GetExpression(root, string, arr, pos));

        if (**string == ')') {
            (*string)++;
        } else {
            fprintf(stderr, "SYNTAX_ERROR_P: expected ')', got '%c'", **string);
            return NULL;
        }
        return val;

    }
    DifNode_t *value_number = GetNumber(root, string);
    
    if (value_number) {
        return value_number;
    }

    return GetString(root, string, arr, pos);
}

static DifNode_t *GetAssignment(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    const char *save = *string;
    DifNode_t *maybe_var = GetString(root, string, arr, pos);
    if (!maybe_var) {
        *string = save;
        return NULL;
    }

    if (**string != '=') {
        *string = save;
        return NULL;
    }

    (*string)++;

    CHECK_NULL_RETURN(value, GetExpression(root, string, arr, pos)); // присваивание значения ноде
    printf("op done\n");
    return NEWOP(kOperationIs, maybe_var, value);
}

static DifNode_t *GetIf(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    if (!(**string == 'i' && *(*string + 1) == 'f')) return NULL;
    (*string) += 2;

    if (**string != '(') {
        fprintf(stderr, "SYNTAX_ERROR_IF: expected '(' after if\n");
        return NULL;
    }
    (*string)++;

    CHECK_NULL_RETURN(cond, GetExpression(root, string, arr, pos));

    if (**string != ')') {
        fprintf(stderr, "SYNTAX_ERROR_IF: expected ')'\n");
        return NULL;
    }
    (*string)++;

    if (**string != '{')
        return NULL;

    (*string)++;

    DifNode_t *first = GetOp(root, string, arr, pos);
    if (!first) {
        fprintf(stderr, "SYNTAX_ERROR_IF: expected statements in body\n");
        return NULL;
    }
    DifNode_t *last = first;

    while (**string == ';') {
        (*string)++;
        DifNode_t *next = GetOp(root, string, arr, pos);
        if (!next) {
            fprintf(stderr, "SYNTAX_ERROR_IF: expected statement after ';' inside if-body\n");
            return NULL;
        }

        last = NewNode(root, kOperation, (Value){ .operation = kOperationThen }, last, next, arr);
    }

    if (**string != '}') {
        fprintf(stderr, "SYNTAX_ERROR_IF: expected '}' at end of if-body\n");
        return NULL;
    }
    (*string)++;

    return NewNode(root, kOperation, (Value){ .operation = kOperationIf }, cond, last, arr);
}

static DifNode_t *GetWhile(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    if (!(**string == 'w' && *(*string + 1) == 'h' && *(*string + 2) == 'i' && *(*string + 3) == 'l' && *(*string + 4) == 'e'))
        return NULL;

    (*string) += 5;

    if (**string != '(') {
        fprintf(stderr, "SYNTAX_ERROR_WHILE: expected '(' after while\n");
        return NULL;
    }
    (*string)++;

    CHECK_NULL_RETURN(cond, GetExpression(root, string, arr, pos));

    if (**string != ')') {
        fprintf(stderr, "SYNTAX_ERROR_WHILE: expected ')'\n");
        return NULL;
    }
    (*string)++;

    if (**string == '{') {
        (*string)++;
        DifNode_t *first = GetOp(root, string, arr, pos);
        if (!first) {
            fprintf(stderr, "SYNTAX_ERROR_WHILE: expected statements in body\n");
            return NULL;
        }

        DifNode_t *last = first;

        while (**string == ';') {
            (*string)++;
            DifNode_t *next = GetOp(root, string, arr, pos);
            if (!next) {
                fprintf(stderr, "SYNTAX_ERROR_WHILE: expected statement after ';'\n");
                return NULL;
            }
            last = NewNode(root, kOperation, (Value){ .operation = kOperationThen }, last, next, arr);
        }

        if (**string != '}') {
            fprintf(stderr, "SYNTAX_ERROR_WHILE: expected '}'\n");
            return NULL;
        }
        (*string)++;

        return NEWOP(kOperationWhile, cond, last);
    }
    return NULL;

}

static DifNode_t *GetOp(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    const char *save = *string;

    DifNode_t *whilenode = GetWhile(root, string, arr, pos);
    if (whilenode) return whilenode;

    *string = save;
    DifNode_t *ifnode = GetIf(root, string, arr, pos);
    if (ifnode) return ifnode;

    *string = save;
    DifNode_t *assign = GetAssignment(root, string, arr, pos);
    if (assign) return assign;

    *string = save;
    return NULL;
}

static DifNode_t *GetPower(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    CHECK_NULL_RETURN(val, GetPrimary(root, string, arr, pos));

    while (**string == '^') {
        (*string)++;
        CHECK_NULL_RETURN(val2, GetPower(root, string, arr, pos));
        val = POW_(val, val2);
    }

    return val;
}

static DifNode_t *GetNumber(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    DifNode_t *val = NULL;
    NodeCtor(&val, 0);
    if (!val) {
        return NULL;
    }

    val->type = kNumber;
    const char *last_string = *string;

    while ('0' <= **string && **string <= '9') {
        val->value.number = val->value.number * 10 + (**string - '0');
        (*string)++;
    }

    if (last_string == *string) {
        return NULL;
    }
    return val;
}

static DifNode_t *GetString(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    char *buf = (char *) calloc (MAX_TEXT_SIZE, sizeof(char));
    if (!buf) {
        fprintf(stderr, "ERROR no memory.\n");
        return NULL;
    }
    size_t position = 0;

    const char *last_string = *string;
    DifNode_t *op_node = NULL;

    while ( (('a' <= **string && **string <= 'z') ||
             ('A' <= **string && **string <= 'Z') ||
             **string == '_' ||
             ('0' <= **string && **string <= '9')) ) {

        if (position == 0 && ('0' <= **string && **string <= '9')) break;

        buf[position++] = **string;
        buf[position] = '\0';

        (*string)++;

        fprintf(stderr, "buf: %s, string: %s\n", buf, *string);
        op_node = GetOperation(root, string, arr, pos, buf);
        if (op_node) {
            free(buf);
            return op_node;
        }
    }

    if (last_string == *string) {
        free(buf);
        return NULL;
    }

    DifNode_t *var_node = NULL;
    NodeCtor(&var_node, 0);
    if (!var_node) {
        free(buf);
        return NULL;
    }

    var_node->type = kVariable;

    bool flag_found = false;
    Value val = {};

    for (size_t i = 0; i < *pos; i++) {
        if (strcmp(arr->var_array[i].variable_name, buf) == 0) {
            val.variable = &arr->var_array[i];
            var_node->value = val;
            flag_found = true;
            break;
        }
    }

    ResizeArray(arr);

    if (!flag_found) {
        arr->var_array[*pos].variable_name = strdup(buf);
        arr->var_array[*pos].variable_value = 0.0;
        val.variable = &arr->var_array[*pos];
        arr->size ++;
        var_node->value = val;
        (*pos)++;
        free(buf);
    } else {
        free(buf);
    }

    return var_node;
}


static DifNode_t *GetOperation(DifRoot *root, const char **string, VariableArr *arr, size_t *position, char *buffer) {
    assert(root);
    assert(string);
    assert(arr);
    assert(position);

    OperationTypes type = kOperationNone;

    size_t pos = 0;
    const char *last_position = *string;

    type = ParseOperator(buffer);
    if (type != kOperationNone) {
        //(*string)++;
        //free(buf);
        if (**string == '(') {
            (*string)++;
            DifNode_t *new_node = GetExpression(root, string, arr, position); //
            if (**string == ')') {
                (*string)++;
                return NewNode(root, kOperation, (Value){ .operation = type}, NULL, new_node, arr);
            }
        }
    }
    
    return NULL;
}

#undef NEWN
#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef POW_



static OperationTypes ParseOperator(const char *string) {
    assert(string);

    size_t op_size = sizeof(operations)/sizeof(operations[0]);

    for (size_t i = 0; i < op_size; i++) {
        if (strncmp(string, operations[i].name, strlen(operations[i].name)) == 0) { //
            return operations[i].type;
        }
    }

    return kOperationNone;
}

static long long SizeOfFile(const char *filename) {
    assert(filename);

    struct stat stbuf = {};

    int err = stat(filename, &stbuf);
    if (err != kSuccess) {
        perror("stat() failed");
        return kErrorStat;
    }

    return stbuf.st_size;
}

static char *ReadToBuf(const char *filename, FILE *file, size_t filesize) {
    assert(filename);
    assert(file);

    char *buf_in = (char *) calloc (filesize + 2, sizeof(char));
    if (!buf_in) {
        fprintf(stderr, "ERROR while calloc.\n");
        return NULL;
    }

    size_t bytes_read = fread(buf_in, 1, filesize, file);

    char *buf_out = (char *) calloc (bytes_read + 1, 1);
    if (!buf_out) {
        fprintf(stderr, "ERROR while calloc buf_out.\n");
        free(buf_in);
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (buf_in[i] != '\n' && buf_in[i] != ' ') {
            buf_out[j++] = buf_in[i];
        }
    }

    buf_out[j] = '\0';

    free(buf_in);

    return buf_out;
}

static void DoBufRead(FILE *file, const char *filename, FileInfo *Info) {
    assert(file);
    assert(filename);
    assert(Info);

    Info->filesize = (size_t)SizeOfFile(filename) * 4;

    Info->buf_ptr = ReadToBuf(filename, file, Info->filesize);
    assert(Info->buf_ptr != NULL);
}