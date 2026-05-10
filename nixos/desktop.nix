# desktop.nix

{ config, pkgs, ... }:

{
  services.xserver.enable = true;

  services.displayManager.sddm = {
    enable = true;
    wayland.enable = true;
    theme = "forest";
  };
  environment.systemPackages = with pkgs; [
    kdePackages.qtgraphicaleffects
    kdePackages.qtsvg
    kdePackages.qtmultimedia
  ];
  services.desktopManager.plasma6.enable = true;
  programs.dconf.enable = true;
  environment.plasma6.excludePackages = [ ];

  /*
    environment.sessionVariables = {
      NIXOS_OZONE_WL = "1";
      WLR_NO_HARDWARE_CURSORS = "1";
      LIBVA_DRIVER_NAME = "nvidia";
      GBM_BACKEND = "nvidia-drm";
      __GLX_VENDOR_LIBRARY_NAME = "nvidia";
      WLR_DRM_NO_ATOMIC = "1";
      KWIN_DRM_NO_AMS = "1";
    };
  */

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

  services.blueman.enable = true;
  services.printing.enable = true;
}
