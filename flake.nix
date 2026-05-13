# flake.nix
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
    pkgs = nixpkgs.legacyPackages.${system};
  in
  {
    nixpkgs.overlays = [
      (final: prev: {
        wlroots = prev.wlroots.overrideAttrs (old: {
          patches = (old.patches or []) ++ [
            (final.writeText "wlroots-fix.patch" ''
              diff --git a/backend/libinput/switch.c b/backend/libinput/switch.c
              @@ -30,6 +30,7 @@
               switch (libinput_event_switch_get_switch(sevent)) {
                   case LIBINPUT_SWITCH_LID:
                   case LIBINPUT_SWITCH_TABLET_MODE:
+                  case LIBINPUT_SWITCH_KEYPAD_SLIDE:
                       break;
               }
            '')
          ];
        });
      })
    ];

    nixosConfigurations.naxce = nixpkgs.lib.nixosSystem {
      inherit system;

      specialArgs = {
        repoPath = ./.;
        zenithwm = import ./nixos/ZenithWM/nix/default.nix { inherit pkgs; };
      };

      modules = [
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