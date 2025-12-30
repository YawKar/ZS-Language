#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <stdio.h>

#include <cstddef>
#include <cstdio>

#include "Common/Enums.h"
#include "Common/IO.h"
#include "Common/Structs.h"
#include "External/doctest/doctest.h"
#include "FrontEnd/LanguageFunctions.h"
#include "FrontEnd/Rules.h"
#include "FrontEnd/TreeToAsm.h"

#define REQUIRE_SUCCESS(cond) REQUIRE((cond) == kSuccess);

#define REQUIRE_OPEN_FILE(file, filename, mode) \
    FILE* file = fopen(filename, mode);         \
    REQUIRE((file) != nullptr);

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MAX_TEST_RESOURCE_BUFFER_SIZE_BYTES 16384

const char* const RESOURCES_DIR = "build/tests/FrontEnd/resources/";

TEST_CASE("Testing conversion from code to tree and asm") {
    const char* test_cases[2] = {
        "code_factorial",
        "code_square",
    };

    for (size_t test_case_ix = 0; test_case_ix < ARRAY_LENGTH(test_cases);
         test_case_ix++) {
        SUBCASE(test_cases[test_case_ix]) {
            char filename_input[255];
            sprintf(
                filename_input, "%s%s", RESOURCES_DIR, test_cases[test_case_ix]
            );

            SUBCASE("conversion to tree") {
                LangRoot root = {};
                LangRootCtor(&root);

                INIT_DUMP_INFO(dump_info);
                dump_info.tree = &root;

                VariableArr variable_array = {};
                REQUIRE_SUCCESS(InitArrOfVariable(&variable_array, 16));

                Language lang_info = {
                    .root = &root,
                    .tokens = NULL,
                    .tokens_pos = NULL,
                    .arr = &variable_array
                };

                char actual_tree_printed[MAX_TEST_RESOURCE_BUFFER_SIZE_BYTES];

                // Doing the job
                FILE* mem_stream = fmemopen(
                    actual_tree_printed, sizeof(actual_tree_printed), "w"
                );
                REQUIRE(mem_stream != nullptr);
                REQUIRE_SUCCESS(
                    ReadInfix(&lang_info, &dump_info, filename_input, false)
                );
                PrintAST(root.root, mem_stream, &variable_array, 0);
                fclose(mem_stream);
                DtorVariableArray(&variable_array);

                // Getting expected tree
                char filename_tree_expected[255];
                sprintf(
                    filename_tree_expected,
                    "%s%s.tree.expected",
                    RESOURCES_DIR,
                    test_cases[test_case_ix]
                );
                REQUIRE_OPEN_FILE(expected_file, filename_tree_expected, "r");
                const char* expected = ReadToBuf(
                    filename_tree_expected,
                    expected_file,
                    (size_t)SizeOfFile(filename_tree_expected)
                );
                fclose(expected_file);

                REQUIRE(actual_tree_printed == expected);
            }

            SUBCASE("conversion to asm") {
                LangRoot root = {};
                LangRootCtor(&root);

                INIT_DUMP_INFO(dump_info);
                dump_info.tree = &root;

                VariableArr variable_array = {};
                REQUIRE_SUCCESS(InitArrOfVariable(&variable_array, 16));

                Language lang_info = {
                    .root = &root,
                    .tokens = NULL,
                    .tokens_pos = NULL,
                    .arr = &variable_array
                };

                char actual_asm_printed[MAX_TEST_RESOURCE_BUFFER_SIZE_BYTES];

                // Doing the job
                FILE* mem_stream = fmemopen(
                    actual_asm_printed, sizeof(actual_asm_printed), "w"
                );
                REQUIRE(mem_stream != nullptr);
                REQUIRE_SUCCESS(
                    ReadInfix(&lang_info, &dump_info, filename_input, false)
                );
                int ram_base = 0;
                PrintProgram(mem_stream, root.root, &variable_array, &ram_base);
                fclose(mem_stream);
                DtorVariableArray(&variable_array);

                // Getting expected asm
                char filename_asm_expected[255];
                sprintf(
                    filename_asm_expected,
                    "%s%s.asm.expected",
                    RESOURCES_DIR,
                    test_cases[test_case_ix]
                );
                REQUIRE_OPEN_FILE(expected_file, filename_asm_expected, "r");
                const char* expected = ReadToBuf(
                    filename_asm_expected,
                    expected_file,
                    (size_t)SizeOfFile(filename_asm_expected)
                );
                fclose(expected_file);

                REQUIRE(actual_asm_printed == expected);
            }
        }
    }
}
