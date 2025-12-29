{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" "aarch64-darwin" ];

      perSystem =
        { system, pkgs, ... }:
        let
          gccVersion = "gcc15";
          gccToolchain = pkgs."${gccVersion}";
          gccStdenv = pkgs."${gccVersion}Stdenv";
        in
        {
          devShells.default = pkgs.mkShell.override { stdenv = gccStdenv; } {
            name = "Reproducible development environment";

            packages = with pkgs; [
              meson
              ninja
              pkg-config
              nixfmt-rfc-style
              gnumake
            ] ++ [
              gccToolchain
            ];

            shellHook = ''
              export PS1=[Language]$PS1
              echo ""
              echo "Development shell loaded!"
              echo "  system: ${system}"
              echo "  g++:    $(g++ -dumpversion)"
              echo "  gcc:    $(gcc -dumpversion)"
              echo "  make:   $(make --version | head -1)
              echo "  meson:  $(meson --version)"
            '';
          };
        };
    };
}
