{ config, pkgs, ... }:

{
  services.xserver.enable = true;

  services.displayManager.sddm = {
    enable = true;
    wayland.enable = true;
    theme = "forest";
  };

  services.desktopManager.plasma6.enable = true;
  programs.dconf.enable = true;

  nixpkgs.overlays = [
    (final: prev: {
      wlroots = prev.wlroots.overrideAttrs (old: {
        NIX_CFLAGS_COMPILE =
          (old.NIX_CFLAGS_COMPILE or "") + " -Wno-error=switch";
      });
    })
  ];

  services.displayManager.sessionPackages = [
    (pkgs.writeTextDir "share/wayland-sessions/zenithwm.desktop" ''
      [Desktop Entry]
      Name=ZenithWM
      Comment=Zenith Wayland Compositor
      Exec=zenithwm
      Type=Application
    '')
  ];

  environment.systemPackages = with pkgs; [
    kdePackages.qt5compat
    kdePackages.qtsvg
    kdePackages.qtmultimedia

    zenithwm

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

  programs.firefox = {
    enable = true;
    package = pkgs.firefox;
  };

  services.printing.enable = true;
}