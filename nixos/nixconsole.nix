{ config, pkgs, ... }:

{
  system.nixos.tags = [ "NixConsole" ];

  services.displayManager.sddm.enable = false;

  services.getty.autologinUser = "naxce";

  programs.gamescope.enable = true;
  programs.steam.enable = true;
  programs.gamemode.enable = true;

  environment.systemPackages = with pkgs; [
    mangohud
  ];

  environment.loginShellInit = ''
    if [[ "$(tty)" == "/dev/tty1" ]]; then
      exec ${pkgs.gamescope}/bin/gamescope -f -- \
        --mangoapp \
        ${pkgs.steam}/bin/steam -gamepadui
    fi
  '';

  powerManagement.cpuFreqGovernor = "performance";
}