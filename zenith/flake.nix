{
  description = "ZenithDE - minimal gaming desktop environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      forAllSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
    in
    {
      packages = forAllSystems (system: {
        zenithde = nixpkgs.legacyPackages.${system}.callPackage ./default.nix { };
        default = self.packages.${system}.zenithde;
      });

      homeManagerModules.zenith = import ./hm-module.nix self;
    };
}
