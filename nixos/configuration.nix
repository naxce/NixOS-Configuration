{ config, pkgs, ... }:

{
  imports = [
    ./hardware-configuration.nix
    ./network.nix
    ./core.nix
    ./packages.nix
    ./desktop.nix
    ./gaming.nix
    ./users.nix
  ];

  nixpkgs.config.allowUnfree = true;
  system.stateVersion = "26.05";
}
