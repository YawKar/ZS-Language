subdir('resources')

front_end_tests_sources = files('test.cpp')
front_end_tests_lib = library(
    'front_end_tests',
    front_end_tests_sources,
    link_with: [common_lib, front_end_lib, reverse_end_lib],
    include_directories: [libs_include],
)
