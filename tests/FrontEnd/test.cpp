#include <cstring>
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <dirent.h>
#include <regex.h>
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

const char* const RESOURCES_DIR = "./tests/FrontEnd/resources/";
const char* const CODE_SOURCE_REGEX = "^code_[[:alnum:]]+$";

TEST_CASE("Testing conversion from code to tree") {
    regex_t code_source_regex;
    REQUIRE(regcomp(&code_source_regex, CODE_SOURCE_REGEX, REG_EXTENDED) == 0);

    DIR* resources = opendir(RESOURCES_DIR);
    REQUIRE(resources != nullptr);

    struct dirent* dp;
    while ((dp = readdir(resources)) != nullptr) {
        // Skip '..' and '.'
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }
        // Skip unmatched
        int reti = regexec(&code_source_regex, dp->d_name, 0, NULL, 0);
        if (reti == REG_NOMATCH) {
            continue;
        } else if (reti != 0) {
            char errmsg[200];
            regerror(reti, &code_source_regex, errmsg, sizeof(errmsg));
            regfree(&code_source_regex);
            FAIL("Regex match failed: %s", errmsg);
        }
        // Now we got resource
        char filename_input[300];
        sprintf(filename_input, "%s%s", RESOURCES_DIR, dp->d_name);

        SUBCASE(dp->d_name) {
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
            FILE* mem_stream =
                fmemopen(actual_tree_printed, sizeof(actual_tree_printed), "w");
            REQUIRE(mem_stream != nullptr);
            REQUIRE_SUCCESS(
                ReadInfix(&lang_info, &dump_info, filename_input, false)
            );
            PrintAST(root.root, mem_stream, &variable_array, 0);
            fclose(mem_stream);
            DtorVariableArray(&variable_array);

            // Getting expected tree
            char filename_tree_expected[300];
            sprintf(
                filename_tree_expected,
                "%s%s.tree.expected",
                RESOURCES_DIR,
                dp->d_name
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
    }
    regfree(&code_source_regex);
}

TEST_CASE("Testing conversion from code to asm") {
    regex_t code_source_regex;
    REQUIRE(regcomp(&code_source_regex, CODE_SOURCE_REGEX, REG_EXTENDED) == 0);

    DIR* resources = opendir(RESOURCES_DIR);
    REQUIRE(resources != nullptr);

    struct dirent* dp;
    while ((dp = readdir(resources)) != nullptr) {
        // Skip '..' and '.'
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }
        // Skip unmatched
        int reti = regexec(&code_source_regex, dp->d_name, 0, NULL, 0);
        if (reti == REG_NOMATCH) {
            continue;
        } else if (reti != 0) {
            char errmsg[200];
            regerror(reti, &code_source_regex, errmsg, sizeof(errmsg));
            regfree(&code_source_regex);
            FAIL("Regex match failed: %s", errmsg);
        }
        // Now we got resource
        char filename_input[300];
        sprintf(filename_input, "%s%s", RESOURCES_DIR, dp->d_name);

        SUBCASE(dp->d_name) {
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
            FILE* mem_stream =
                fmemopen(actual_asm_printed, sizeof(actual_asm_printed), "w");
            REQUIRE(mem_stream != nullptr);
            REQUIRE_SUCCESS(
                ReadInfix(&lang_info, &dump_info, filename_input, false)
            );
            int ram_base = 0;
            AsmInfo asm_info = {};
            PrintProgram(
                mem_stream, root.root, &variable_array, &ram_base, &asm_info
            );
            fclose(mem_stream);
            DtorVariableArray(&variable_array);

            // Getting expected asm
            char filename_asm_expected[300];
            sprintf(
                filename_asm_expected,
                "%s%s.asm.expected",
                RESOURCES_DIR,
                dp->d_name
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
    regfree(&code_source_regex);
}
