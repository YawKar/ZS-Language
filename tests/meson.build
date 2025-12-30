subdir('FrontEnd')
subdir('MiddleEnd')
subdir('ReverseEnd')

tests_exe = executable(
    'tests_exe',
    'tests.cpp',
    link_whole: [
        front_end_tests_lib,
        middle_end_tests_lib,
        reverse_end_tests_lib,
    ],
    include_directories: [libs_include],
)
