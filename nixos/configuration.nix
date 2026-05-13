# configuration.nix
{ config, pkgs, ... }:

{
  imports = [
    ./hardware-configuration.nix
    ./network.nix
    ./core.nix
    ./desktop.nix
    ./gaming.nix
    ./users.nix
  ];

  system.stateVersion = "26.05";
  nixpkgs.config.allowUnfree = true;
}
