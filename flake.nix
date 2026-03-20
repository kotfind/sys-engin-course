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
          ccls
          clang
          cmake
          mold
        ];
      };
    in {
      devShells.default = shell;
    });
}
