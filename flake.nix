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
  };

  outputs = { self, nixpkgs, home-manager, plasma-manager, ... }:
  let
    system = "x86_64-linux";
    
    wlroots-overlay = (final: prev: {
      wlroots = prev.wlroots.overrideAttrs (old: {
        patches = (old.patches or []) ++ [
          (final.writeText "wlroots-fix.patch" ''
            --- a/backend/libinput/switch.c
            +++ b/backend/libinput/switch.c
            @@ -30,6 +30,7 @@
                     switch (libinput_event_switch_get_switch(sevent)) {
                         case LIBINPUT_SWITCH_LID:
                         case LIBINPUT_SWITCH_TABLET_MODE:
            +            case LIBINPUT_SWITCH_KEYPAD_SLIDE:
                             break;
                     }
            @@ -60,6 +61,7 @@
                     switch (libinput_event_switch_get_switch(sevent)) {
                         case LIBINPUT_SWITCH_LID:
                         case LIBINPUT_SWITCH_TABLET_MODE:
            +            case LIBINPUT_SWITCH_KEYPAD_SLIDE:
                             break;
                     }
          '')
        ];
      });
    });

    pkgs = import nixpkgs {
      inherit system;
      overlays = [ wlroots-overlay ];
      config.allowUnfree = true;
    };
  in
  {
    nixosConfigurations.naxce = nixpkgs.lib.nixosSystem {
      inherit system;

      specialArgs = {
        inherit (self) inputs;
        repoPath = ./.;
        zenithwm = import ./nixos/ZenithWM/nix/default.nix { inherit pkgs; };
      };

      modules = [
        { nixpkgs.overlays = [ wlroots-overlay ]; }
        
        ./nixos/configuration.nix

        home-manager.nixosModules.home-manager
        {
          home-manager.useGlobalPkgs = true;
          home-manager.useUserPackages = true;

          home-manager.extraSpecialArgs = {
            repoPath = ./.;
          };

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