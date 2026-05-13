{
  description = "naxce nixos config";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

    home-manager = {
      url = "github:nix-community/home-manager";
      inputs.nixpkgs.follows = "nixpkgs";
    };

    plasma-manager = {
      url = "github:nix-community/plasma-manager";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.home-manager.follows = "home-manager";
    };

    zenith = {
      url = "path:/home/naxce/dotfiles/zenith";
      #url = "github:naxce/zenith";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      home-manager,
      plasma-manager,
      zenith,
      ...
    }:
    let
      system = "x86_64-linux";

      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };
    in
    {
      nixosConfigurations.naxce = nixpkgs.lib.nixosSystem {
        inherit system;
        specialArgs = { inherit zenith; };
        modules = [
          ./nixos/configuration.nix
          zenith.nixosModules.default

          home-manager.nixosModules.home-manager
          {
            home-manager.useGlobalPkgs = true;
            home-manager.useUserPackages = true;
            home-manager.extraSpecialArgs = { inherit zenith; };
            home-manager.users.naxce = {
              imports = [
                ./home.nix
                ./nixos/packages.nix
                plasma-manager.homeModules.plasma-manager
              ];
            };
          }
        ];
      };
    };
}
