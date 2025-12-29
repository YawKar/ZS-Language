reverse_end_exe_sources = files('main.cpp')
reverse_end_exe = executable(
    'reverse_end_exe',
    reverse_end_exe_sources,
    link_with: [common_lib, front_end_lib, reverse_end_lib],
    include_directories: [libs_include],
)
