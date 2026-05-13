{ config, pkgs, zenith, ... }:

{
  services.xserver.enable = true;

  services.displayManager.sessionPackages = [
    zenith.packages.x86_64-linux.default
  ];

  programs.zenith = {
    enable = true;
    nvidia = true;
    terminal = "foot";
  };
  

  services.displayManager.sddm = {
    enable = true;
    wayland.enable = true;
    theme = "forest";
  };

  services.desktopManager.plasma6.enable = true;
  programs.dconf.enable = true;

  environment.systemPackages = with pkgs; [
    kdePackages.qt5compat
    kdePackages.qtsvg
    kdePackages.qtmultimedia

    (pkgs.stdenv.mkDerivation {
      name = "sddm-theme-forest";
      src = pkgs.fetchFromGitHub {
        owner = "Darkkal44";
        repo = "qylock";
        rev = "main";
        sha256 = "sha256-u1+0dkL4gYyIQP/Ap2cGyf6WhQbUNHxDQDkxT/gbZ1Q=";
      };
      installPhase = ''
        mkdir -p $out/share/sddm/themes
        cp -r themes/forest $out/share/sddm/themes/forest
      '';
    })
  ];

  programs.firefox.enable = true;
  services.printing.enable = true;
}
