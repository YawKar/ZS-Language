code_resources_with_expected_outputs = ['code_factorial', 'code_square']

foreach code_resource : code_resources_with_expected_outputs
    resources_to_copy = [
        code_resource,
        code_resource + '.tree.expected',
        code_resource + '.asm.expected',
    ]
    foreach resource : resources_to_copy
        configure_file(input: resource, output: resource, copy: true)
    endforeach
endforeach
