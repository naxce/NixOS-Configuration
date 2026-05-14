{ config, pkgs, ... }:

let
  zenith = (builtins.getFlake "path:/path/to/zenith").packages.${pkgs.system}.zenith;
in
{
  imports = [
    (builtins.getFlake "path:/path/to/zenith").nixosModules.default
  ];

  programs.zenith = {
    enable = true;
    nvidia = true;
    terminal = "kitty";
  };

  environment.systemPackages = with pkgs; [
    kitty
    xdg-utils
  ];

  services.displayManager.sddm = {
    enable = true;
    wayland.enable = true;
  };

  hardware.nvidia = {
    modesetting.enable = true;
    open = false;
  };

  boot.kernelParams = [
    "nvidia-drm.modeset=1"
    "nvidia-drm.fbdev=1"
  ];
}
