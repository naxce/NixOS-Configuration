
{
  description = "Zenith - minimal gaming desktop environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    forAllSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
  in {
    packages = forAllSystems (system: {
      zenith = nixpkgs.legacyPackages.${system}.callPackage ./default.nix {};
      default = self.packages.${system}.zenith;
    });

    homeManagerModules.zenith = import ./hm-module.nix self;
  };
}