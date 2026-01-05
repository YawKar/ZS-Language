[private]
default:
    @echo "Variables:"
    @just --evaluate | xargs -I SUBST printf "    %s\n" "SUBST"
    @just --list --unsorted

build_dir := "build"
asan_and_ubsan := "address,undefined"
tsan_and_ubsan := "thread,undefined"
default_sanitizers := asan_and_ubsan
default_debugger := "gef"

[group("FrontEnd")]
front-build sanitizers=default_sanitizers: (build "front_end_exe" sanitizers)

[group("FrontEnd")]
front-run *ARGS: (run "front_end_exe" default_sanitizers ARGS)

[group("FrontEnd")]
front-run-with-custom-sanitizers sanitizers *ARGS: (run "front_end_exe" sanitizers ARGS)

[group("FrontEnd")]
front-tests-run sanitizers=default_sanitizers *ARGS: (run "tests_exe" sanitizers '-sf="*FrontEnd*"' ARGS)

[group("FrontEnd")]
front-tests-debug sanitizers=default_sanitizers *ARGS: (debug "tests_exe" sanitizers '-sf="*FrontEnd*"' ARGS)

[group("FrontEnd")]
[script]
front-tests-regenerate: (front-build "none")
    executable=$(just find_executable front_end_exe none)
    for source_resource in $(find ./tests/FrontEnd/resources -type f -not -iname "*expected*"); do
      $executable code-tree "$source_resource" "$source_resource.tree.expected"
      $executable code-asm "$source_resource" "$source_resource.asm.expected"
    done

[group("MiddleEnd")]
middle-build sanitizers=default_sanitizers: (build "middle_end_exe" sanitizers)

[group("MiddleEnd")]
middle-run *ARGS: (run "middle_end_exe" default_sanitizers ARGS)

[group("MiddleEnd")]
middle-run-with-custom-sanitizers sanitizers *ARGS: (run "middle_end_exe" sanitizers ARGS)

[group("MiddleEnd")]
middle-tests-run sanitizers=default_sanitizers: (run "tests_exe" sanitizers '-sf="*MiddleEnd*"')

[group("ReverseEnd")]
reverse-build sanitizers=default_sanitizers: (build "reverse_end_exe" sanitizers)

[group("ReverseEnd")]
reverse-run *ARGS: (run "reverse_end_exe" default_sanitizers ARGS)

[group("ReverseEnd")]
reverse-run-with-custom-sanitizers sanitizers *ARGS: (run "reverse_end_exe" sanitizers ARGS)

[group("ReverseEnd")]
reverse-tests-run sanitizers=default_sanitizers: (run "tests_exe" sanitizers '-sf="*ReverseEnd*"')

[group("All")]
all-build:
    @just --summary | grep -Eo "\w+-build" | grep -v "all-build" | xargs -I SUBST just SUBST

[group("All")]
all-tests-run sanitizers=default_sanitizers: (run "tests_exe" sanitizers)

[group("Meta")]
fmt only_check="false":
    just --unstable --fmt
    nixfmt -s -v flake.nix
    meson format -r {{ if only_check == "false" { "-i" } else { "--check-only" } }}
    find bins libs tests -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --Werror {{ if only_check == "false" { "-i" } else { "--dry-run" } }}

[group("Meta")]
lint only_check="false" sanitizers=default_sanitizers: (ensure_build_dir_setup sanitizers)
    run-clang-tidy -quiet -p {{ build_dir / sanitizers }} {{ if only_check == "false" { "-fix" } else { "" } }} bins libs tests

[group("Meta")]
[private]
[script]
ensure_build_dir_setup sanitizers:
    specific_build_dir={{ build_dir / sanitizers }}
    if [ ! -d $specific_build_dir ]; then
      mkdir -p $specific_build_dir
      meson setup -Db_sanitize={{ sanitizers }} $specific_build_dir
    fi

[group("Meta")]
build meson_target sanitizers: (ensure_build_dir_setup sanitizers)
    @meson compile -C {{ build_dir / sanitizers }} {{ meson_target }}
    @echo "Compiled executable: $(just find_executable {{ meson_target }} {{ sanitizers }})"

[group("Meta")]
run meson_target sanitizers *ARGS: (build meson_target sanitizers)
    @just center_header "EXECUTING"
    @$(just find_executable {{ meson_target }} {{ sanitizers }}) {{ ARGS }}

[group("Meta")]
debug meson_target sanitizers *ARGS: (build meson_target sanitizers)
    @just center_header "DEBUGGING"
    @{{ default_debugger }} --args $(just find_executable {{ meson_target }} {{ sanitizers }}) {{ ARGS }}

[group("Utils")]
[private]
find_executable meson_target sanitizers:
    @# using `realpath` to transform long absolute path to the relative to the current working directory
    @realpath --relative-to=. "$(meson introspect {{ build_dir / sanitizers }} --targets | jq -r '.[] | select(.name == "{{ meson_target }}") | .filename[0]')"

# `just center_header LOL` prints LOL in the middle of the row
[group("Utils")]
[private]
[script]
center_header text filler="_":
    text="{{ text }}"
    width=$(tput cols)
    pad=$(( (width - ${#text}) / 2 ))
    printf '%*s' "$pad" '' | tr ' ' '{{ filler }}'
    printf '%s' "$text"
    printf '%*s\n' "$(( width - pad - ${#text} ))" '' | tr ' ' '{{ filler }}'
