#include <cstddef>
#include <cstdio>

#include "Common/DoGraph.h"
#include "Common/Enums.h"
#include "Common/IO.h"
#include "Common/Structs.h"
#include "FrontEnd/LanguageFunctions.h"
#include "FrontEnd/ReadTree.h"
#include "FrontEnd/Rules.h"
#include "ReverseEnd/TreeToCode.h"

int main(int, char* argv[]) {
    char* tree_file = argv[1];
    char* code_file = argv[2];

    VariableArr Variable_Array = {};
    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(InitArrOfVariable(&Variable_Array, 10));

    INIT_DUMP_INFO(dump_info);

    FILE_OPEN_AND_CHECK(ast_file, tree_file, "r");

    FileInfo info = {};
    DoBufRead(ast_file, tree_file, &info);
    fclose(ast_file);

    LangRoot parsed_root = {};
    LangRootCtor(&parsed_root);

    size_t pos = 0;
    LangNode_t* tree = NULL;

    CHECK_ERROR_RETURN(
        ParseNodeFromString(info.buf_ptr, &pos, NULL, &tree, &Variable_Array)
    );

    // fprintf(stderr, "%zu\n\n", variable_array.size);
    // for (size_t i = 0; i < variable_array.size; i++) {
    //     fprintf(stderr, "%s %d\n\n",
    //     variable_array.var_array[i].variable_name,
    //     variable_array.var_array[i].variable_value);
    // }

    parsed_root.root = tree;
    dump_info.tree = &parsed_root;

    DoTreeInGraphviz(parsed_root.root, &dump_info, &Variable_Array);

    FILE_OPEN_AND_CHECK(code_out, code_file, "w");
    GenerateCodeFromAST(parsed_root.root, code_out, &Variable_Array, 0);
    fclose(code_out);

    DtorVariableArray(&Variable_Array);
    // TreeDtor(&root);

    return kSuccess;
}
