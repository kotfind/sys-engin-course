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
        ];

        hardeningDisable = ["all"];
      };
    in {
      devShells.default = shell;
    });
}
