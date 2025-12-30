# Doctest single-header library: https://github.com/doctest/doctest
# `wget https://github.com/doctest/doctest/releases/download/v2.4.12/doctest.h`
# Here's a simple integration test that checks that doctest can be successfully compiled on the host system.

doctest_example = '''
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "External/doctest/doctest.h"

TEST_CASE("an empty test that will succeed - not part of a test suite") {}

TEST_SUITE("scoped test suite") {
    TEST_CASE("part of scoped") {
        CHECK(2 == 2);
        MESSAGE("part of scoped");
    }

    TEST_CASE("part of scoped 2") {
        MESSAGE("part of scoped 2");
    }
}
'''

result = meson.get_compiler('cpp').run(
    doctest_example,
    name: 'check that doctest compiles',
    include_directories: [libs_include],
)

if not result.compiled()
    error('Failed to compile basic example with doctest.h!')
endif

if result.returncode() != 0
    error(
        'Basic doctest.h example was expected to return 0 but got',
        result.returncode(),
        '.',
    )
endif
