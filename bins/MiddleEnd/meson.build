middle_end_exe_sources = files([
  'main.cpp',
])
middle_end_exe = executable(
  'middle_end',
  middle_end_exe_sources,
  link_with: [
    common_lib,
    front_end_lib,
    middle_end_lib,
    reverse_end_lib,
  ],
  include_directories: [libs_include],
)
