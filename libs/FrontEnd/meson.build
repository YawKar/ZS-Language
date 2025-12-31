front_end_lib_sources = files(
    'LanguageFunctions.cpp',
    'LexicalAnalysis.cpp',
    'ReadTree.cpp',
    'Rules.cpp',
    'TreeToAsm.cpp',
)
front_end_lib = library(
    'front_end_lib',
    front_end_lib_sources,
    link_with: [common_lib],
    include_directories: [libs_include],
)
