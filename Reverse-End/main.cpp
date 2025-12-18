#include "Front-End/Rules.h"
#include "Structs.h"
#include "Enums.h"
#include "Front-End/LanguageFunctions.h"
#include "DoGraph.h"
#include "Front-End/TreeToAsm.h"
#include "TreeToCode.h"
#include "StackFunctions.h"

int main(int argc, char *argv[]) {
    char *tree_file = argv[1];
    char *code_file = argv[2];

    LangRoot root = {};
    LangRootCtor(&root);

    VariableArr Variable_Array = {};
    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(InitArrOfVariable(&Variable_Array, 10));

    INIT_DUMP_INFO(dump_info);
    dump_info.tree = &root;
    Language lang_info = {&root, NULL, NULL, &Variable_Array};
    
    FILE_OPEN_AND_CHECK(ast_file_read, tree_file, "r");
    FileInfo info = {};
    DoBufRead(ast_file_read, tree_file, &info);
    LangRoot root1 = {};
    LangRootCtor(&root1);
    LangNode_t new_node = {};
    LangNode_t *new_node_adr = &new_node;

    size_t pos = 0;
    ParseNodeFromString(info.buf_ptr, &pos, root.root, &new_node_adr, &Variable_Array);
    root1.root = new_node_adr;
    dump_info.tree = &root1;
    DoTreeInGraphviz(root1.root, &dump_info, &Variable_Array);

    FILE_OPEN_AND_CHECK(code_out, code_file, "w");
    GenerateCodeFromAST(root1.root, code_out, &Variable_Array, 0);
    fclose(code_out);
    
    DtorVariableArray(&Variable_Array);
    //TreeDtor(&root);

    return kSuccess;
}