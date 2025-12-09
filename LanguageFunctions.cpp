#include "LanguageFunctions.h"

#include "Enums.h"
#include "Structs.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

size_t DEFAULT_SIZE = 60;

DifErrors DifRootCtor(DifRoot *root) {
    assert(root);

    root->root = NULL;
    root->size = 0;

    return kSuccess;
}

DifErrors NodeCtor(DifNode_t **node, Value *value) {
    assert(node);

    *node = (DifNode_t *) calloc (DEFAULT_SIZE, sizeof(DifNode_t));
    if (!*node) {
        fprintf(stderr, "No memory to calloc NODE.\n");
        return kNoMemory;
    }

    if (value) {
        (*node)->value = *value; 
    } else {
        (*node)->type = kNumber;
        (*node)->value.number = 0;
    }
    
    (*node)->left =  NULL;
    (*node)->right =  NULL;
    (*node)->parent = NULL;

    return kSuccess;
}

DifErrors DeleteNode(DifNode_t *node) {
    if (!node)
        return kSuccess;

    if (node->left) {
        DeleteNode(node->left);
    }

    if (node->right) {
        DeleteNode(node->right);
    }

    node->parent = NULL;

    free(node);

    return kSuccess;
}

DifErrors TreeDtor(DifRoot *tree) {
    assert(tree);

    DeleteNode(tree->root);

    tree->root =  NULL;
    tree->size = 0;

    return kSuccess;
}

DifErrors InitArrOfVariable(VariableArr *arr, size_t capacity) {
    assert(arr);

    arr->capacity = capacity;
    arr->size = 0;

    arr->var_array = (VariableInfo *) calloc (capacity, sizeof(VariableInfo));
    if (!arr->var_array) {
        fprintf(stderr, "Memory error.\n");
        return kNoMemory;
    }

    return kSuccess;
}

DifErrors ResizeArray(VariableArr *arr)  {
    assert(arr);

    if (arr->size + 2 > arr->capacity) {
        VariableInfo *ptr = (VariableInfo *) realloc (arr->var_array, (arr->capacity += 2) * sizeof(VariableInfo));
        if (!ptr) {
            fprintf(stderr, "Memory error.\n");
            return kNoMemory;
        }
        arr->var_array = ptr;
    }

    return kSuccess;
}

DifErrors DtorVariableArray(VariableArr *arr) {
    assert(arr);

    arr->capacity = 0;
    arr->size = 0;

    free(arr->var_array);

    return kSuccess;
}

DifNode_t *NewNode(DifRoot *root, DifTypes type, Value value, DifNode_t *left, DifNode_t *right,
    VariableArr *Variable_Array) {
    assert(root);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    root->size++;
    new_node->type = type;

    switch (type) {

    case kNumber:
        new_node->value.number = value.number;
        break;

    case kVariable: {
        new_node->value = value;
    
        break;
    }

    case kOperation:
        new_node->value.operation = value.operation;
        new_node->left = left;
        new_node->right = right;

        if (left)  left->parent  = new_node;
        if (right) right->parent = new_node;

        break;
    }

    return new_node;
}

DifNode_t *NewVariable(DifRoot *root, const char *variable, VariableArr *VariableArr) {
    assert(root);
    assert(variable);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    root->size ++;
    new_node->type = kVariable;
    VariableInfo *addr = NULL;

    for (size_t i = 0; i < VariableArr->size; i++) {
        if (strncmp(variable, VariableArr->var_array[i].variable_name, strlen(variable)) == 0) {
           addr = &VariableArr->var_array[i];
        }
    }

    if (!addr) {
        ResizeArray(VariableArr);
        VariableArr->var_array[VariableArr->size].variable_name = variable;
        addr = &VariableArr->var_array[VariableArr->size];
        VariableArr->size ++;
    }

        
    new_node->value.variable = addr;

    return new_node;
}

const char *ConvertEnumToOperation(OperationTypes type) {
    switch (type) {
        case kOperationAdd:       return "+";
        case kOperationSub:       return "-";
        case kOperationMul:       return "*";
        case kOperationDiv:       return "/";
        case kOperationPow:       return "^";
        case kOperationSin:       return "sin";
        case kOperationCos:       return "cos";
        case kOperationTg:        return "tg";
        case kOperationLn:        return "ln";
        case kOperationArctg:     return "arctg";
        case kOperationSinh:      return "sh";
        case kOperationCosh:      return "ch";
        case kOperationTgh:       return "th";
        case kOperationIs:        return "=";
        case kOperationIf:        return "if";
        case kOperationElse:      return "else";
        case kOperationWhile:     return "while";
        case kOperationThen:      return ";";
        case kOperationParOpen:   return "(";
        case kOperationParClose:  return ")";
        case kOperationBraceOpen: return "{";
        case kOperationBraceClose:return "}";
        case kOperationNone:      return "none";
    }
    
    return NULL;
}

DifErrors PrintAST(DifNode_t *node, FILE *file) {
    if (!node) {
        fprintf(file, "nil");
        return kSuccess;
    }
    assert(file);

    fprintf(file, "( ");
    
    switch (node->type) {
        case kNumber:
            fprintf(file, "\"%lf\"", node->value.number);
            break;
        case kVariable:
            fprintf(file, "\"%s\"", node->value.variable->variable_name);
            break;
        case kOperation:
            fprintf(file, "\"_%s_\"", ConvertEnumToOperation(node->value.operation));
            break;
        default:
            fprintf(file, "\"UNKNOWN\"");
            break;
    }
    
    fprintf(file, " ");
    PrintAST(node->left, file);
    fprintf(file, " ");
    PrintAST(node->right, file);
    fprintf(file, " )");
    
    return kSuccess;
}