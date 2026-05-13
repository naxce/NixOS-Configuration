# desktop.nix

{ config, pkgs, zenithwm, ... }:

{
  services.xserver.enable = true;

  services.displayManager.sddm = {
    enable = true;
    wayland.enable = true;
    theme = "forest";
  };

  services.displayManager.sessionPackages = [
    zenithwm
  ];

  nixpkgs.overlays = [
    (final: prev: {
      wlroots = prev.wlroots.overrideAttrs (old: {
        NIX_CFLAGS_COMPILE = (old.NIX_CFLAGS_COMPILE or "") + " -Wno-error=switch";
      });
    })
  ];

  environment.systemPackages = with pkgs; [
    kdePackages.qt5compat
    kdePackages.qtsvg
    kdePackages.qtmultimedia
    zenithwm

    (stdenv.mkDerivation {
      name = "sddm-theme-forest";
      src = fetchFromGitHub {
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

  services.desktopManager.plasma6.enable = true;
  programs.dconf.enable = true;
  environment.plasma6.excludePackages = [ ];

  programs.firefox = {
    enable = true;
    package = pkgs.firefox;
    nativeMessagingHosts.packages = [ pkgs.firefoxpwa ];
    policies = {
      DisableTelemetry = true;
      DisableFirefoxStudies = true;
      EnableTrackingProtection = {
        Value = true;
        Locked = true;
        Cryptomining = true;
        Fingerprinting = true;
      };
    };
  };

  services.printing.enable = true;
}
