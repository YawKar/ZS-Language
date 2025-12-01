#include "Rules.h"
#include "Structs.h"
#include "Enums.h"
#include "LanguageFunctions.h"

int main(void) {
    //FILE_OPEN_AND_CHECK(file, "input.txt", "r");
    DifRoot root = {};
    DifRootCtor(&root);

    VariableArr Variable_Array = {};
    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(InitArrOfVariable(&Variable_Array, 4));

    FILE_OPEN_AND_CHECK(out, "diftex.tex", "w");
    // BeginTex(out);

    INIT_DUMP_INFO(dump_info);
    dump_info.tree = &root;
    CHECK_ERROR_RETURN(ReadInfix(&root, &dump_info, &Variable_Array, "input.txt", out));

    // CHECK_ERROR_RETURN(DiffPlay(&Variable_Array, &root, out, &dump_info));
    // EndTex(out);
    // fclose(out);
    // fclose(file);

    DtorVariableArray(&Variable_Array);
    TreeDtor(&root);

    return kSuccess;
}