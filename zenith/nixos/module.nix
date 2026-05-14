{
  config,
  lib,
  pkgs,
  ...
}:

let
  cfg = config.programs.zenith;
  zenithPkg = pkgs.callPackage ../package.nix { };
in
{
  options.programs.zenith = {
    enable = lib.mkEnableOption "Zenith Wayland compositor";

    package = lib.mkOption {
      type = lib.types.package;
      default = zenithPkg;
      description = "The zenith package to use.";
    };

    nvidia = lib.mkOption {
      type = lib.types.bool;
      default = true;
      description = "Enable NVIDIA-specific environment variables.";
    };

    terminal = lib.mkOption {
      type = lib.types.str;
      default = "kitty";
      description = "Terminal emulator to use.";
    };
  };

  config = lib.mkIf cfg.enable {
    environment.systemPackages = [ cfg.package ];

    services.displayManager.sessionPackages = [ cfg.package ];

    services.displayManager.sddm.enable = lib.mkDefault true;
    services.displayManager.sddm.wayland.enable = lib.mkDefault true;

    environment.sessionVariables = lib.mkIf cfg.nvidia {
      LIBVA_DRIVER_NAME = "nvidia";
      GBM_BACKEND = "nvidia-drm";
      __GLX_VENDOR_LIBRARY_NAME = "nvidia";
      WLR_DRM_NO_ATOMIC = "1";
      __GL_YIELD = "USLEEP";
      __GL_MaxFramesAllowed = "1";
      MOZ_ENABLE_WAYLAND = "1";
      QT_QPA_PLATFORM = "wayland";
      SDL_VIDEODRIVER = "wayland";
      CLUTTER_BACKEND = "wayland";
    };

    hardware.nvidia = lib.mkIf cfg.nvidia {
      modesetting.enable = lib.mkDefault true;
      powerManagement.enable = lib.mkDefault false;
      open = lib.mkDefault false;
      nvidiaSettings = lib.mkDefault false;
    };

    boot.kernelParams = lib.mkIf cfg.nvidia [
      "nvidia-drm.modeset=1"
      "nvidia-drm.fbdev=1"
    ];

    security.polkit.enable = true;
    services.dbus.enable = true;
    xdg.portal.enable = true;
  };
}
