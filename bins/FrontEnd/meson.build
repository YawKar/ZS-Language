front_end_exe_sources = files([
  'main.cpp',
])
front_end_exe = executable(
  'front_end',
  front_end_exe_sources,
  link_with: [
    common_lib,
    front_end_lib,
    reverse_end_lib,
  ],
  include_directories: [libs_include],
)
