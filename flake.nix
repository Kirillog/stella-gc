{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "stella-gc";
        version = "0.1.0";
        src = pkgs.lib.cleanSourceWith {
          src = ./.;
          filter = path: _: !(builtins.elem (baseNameOf path) [ "build" "result" ]);
        };
        nativeBuildInputs = [ pkgs.cmake ];
      };

      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [ cmake gcc gdb valgrind ];
      };
    };
}
