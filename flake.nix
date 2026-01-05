{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-darwin"
      ];

      perSystem =
        { system, pkgs, ... }:
        let
          gccVersion = "gcc15";
          gccToolchain = pkgs."${gccVersion}";
          gccStdenv = pkgs."${gccVersion}Stdenv";
          llvmVersion = "21";
          llvmClangToolchain = pkgs."llvmPackages_${llvmVersion}".clang-tools;
          llvmClangUnwrapped = pkgs."llvmPackages_${llvmVersion}".clang-unwrapped;
        in
        {
          devShells.default = pkgs.mkShell.override { stdenv = gccStdenv; } {
            name = "Reproducible development environment";

            packages =
              with pkgs;
              [
                meson
                ninja
                pkg-config
                nixfmt-rfc-style
                graphviz
                just
                jq
                ncurses
                gnugrep
                findutils
                coreutils
                gef
              ]
              ++ [
                gccToolchain
                llvmClangToolchain
                llvmClangUnwrapped
              ];

            # These are needed for `clang-tidy` (and not only) to find GCC's std libs
            NIX_CFLAGS_COMPILE = [
              # general
              "-isystem ${gccStdenv.cc.cc}/include/c++/${gccStdenv.cc.version}"
              # specific for your platform (e.g. bits/c++config.h)
              "-isystem ${gccStdenv.cc.cc}/include/c++/${gccStdenv.cc.version}/${gccStdenv.hostPlatform.config}"
            ];

            shellHook = ''
              export PS1=[Language]$PS1
              echo ""
              echo "Development shell loaded!"
              echo "  system:     ${system}"
              echo "  g++:        $(g++ -dumpversion)"
              echo "  gcc:        $(gcc -dumpversion)"
              echo "  gef:        $(gef --version | head -1)"
              echo "  just:       $(just --version)"
              echo "  graphviz:   $(dot --version 2>&1)"
              echo "  meson:      $(meson --version)"
              echo "  clang-tidy: $(clang-tidy --version | grep -Eo 'version .*')"
            '';
          };
        };
    };
}
