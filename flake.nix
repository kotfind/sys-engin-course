{
  inputs = {
    nixpkgs.url = "nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};

      inherit (pkgs) mkShell;
      inherit (pkgs.stdenv) mkDerivation;

      shell = mkShell {
        name = "sys-engin-course-shell";

        buildInputs = with pkgs; [
          catch2
          ccache
          clang-tools # !!! put before `clang`
          clang
          cmake
          mold

          gdb
          libllvm

          fuse3
          fuse3.dev
          pkg-config

          doxygen
        ];

        hardeningDisable = ["all"];
      };

      app = mkDerivation {
        pname = "fuse-cpu";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [
          catch2
          clang
          cmake
          mold
          fuse3
          fuse3.dev
          doxygen
          pkg-config
        ];

        buildInputs = with pkgs; [
          fuse3
        ];
      };
    in {
      devShells.default = shell;

      packages = {
        default = app;
        fuse-cpu = app;
      };
    });
}
