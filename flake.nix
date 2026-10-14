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

      # `nix flake check` (or `nix build .#checks.x86_64-linux.default -L`)
      # builds the collector and runs the CTest suite in the sandbox.
      checks.${system}.default = self.packages.${system}.default.overrideAttrs (old: {
        pname = "stella-gc-tests";
        doCheck = true;
        checkPhase = ''
          runHook preCheck
          ctest --output-on-failure
          runHook postCheck
        '';
      });

      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [ cmake gcc gdb valgrind ];
      };
    };
}
