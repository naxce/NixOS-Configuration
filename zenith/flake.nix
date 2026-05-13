{
  description = "Zenith - minimal Wayland compositor for gaming";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        zenith = pkgs.callPackage ./package.nix {};
      in {
        packages = {
          default = zenith;
          zenith = zenith;
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            wayland
            wayland-scanner
            libdrm
            mesa
            libinput
            libudev-zero
            cairo
            pixman
            fontconfig
            libseat
            cmake
            pkg-config
            gcc
            gdb
            valgrind
          ];
        };

        apps.default = {
          type = "app";
          program = "${zenith}/bin/zenith";
        };
      }
    ) // {
      nixosModules.default = import ./nixos/module.nix;
      homeManagerModules.default = import ./nixos/home-manager.nix;
    };
}
