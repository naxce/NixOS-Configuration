{ config, lib, pkgs, ... }:

let
  cfg = config.wayland.windowManager.zenith;
  zenithPkg = pkgs.callPackage ../package.nix {};

  configFile = lib.generators.toKeyValue {} (
    {
      terminal   = cfg.settings.terminal;
      gaps       = toString cfg.settings.gaps;
      border_width = toString cfg.settings.borderWidth;
      nvidia_mode  = if cfg.settings.nvidia then "1" else "0";
    }
    // lib.optionalAttrs (cfg.settings.wallpaper != null) {
      wallpaper = cfg.settings.wallpaper;
    }
  );
in {
  options.wayland.windowManager.zenith = {
    enable = lib.mkEnableOption "Zenith Wayland compositor via Home Manager";

    package = lib.mkOption {
      type = lib.types.package;
      default = zenithPkg;
      description = "The zenith package.";
    };

    settings = {
      terminal = lib.mkOption {
        type = lib.types.str;
        default = "foot";
        description = "Default terminal emulator.";
      };

      wallpaper = lib.mkOption {
        type = lib.types.nullOr lib.types.path;
        default = null;
        description = "Path to wallpaper PNG.";
      };

      gaps = lib.mkOption {
        type = lib.types.int;
        default = 0;
        description = "Gap size in pixels around windows.";
      };

      borderWidth = lib.mkOption {
        type = lib.types.int;
        default = 1;
        description = "Window border width in pixels.";
      };

      nvidia = lib.mkOption {
        type = lib.types.bool;
        default = true;
        description = "Apply NVIDIA environment variables.";
      };
    };

    autostart = lib.mkOption {
      type = lib.types.listOf lib.types.str;
      default = [];
      description = "Commands to run on Zenith startup.";
      example = [ "waybar" "dunst" ];
    };
  };

  config = lib.mkIf cfg.enable {
    home.packages = [ cfg.package ];

    xdg.configFile."zenith/zenith.conf".text = configFile;

    xdg.configFile."zenith/autostart".text =
      lib.concatStringsSep "\n" cfg.autostart + "\n";

    home.sessionVariables = lib.mkIf cfg.settings.nvidia {
      LIBVA_DRIVER_NAME         = "nvidia";
      GBM_BACKEND               = "nvidia-drm";
      __GLX_VENDOR_LIBRARY_NAME = "nvidia";
      WLR_DRM_NO_ATOMIC         = "1";
      __GL_YIELD                = "USLEEP";
      __GL_MaxFramesAllowed     = "1";
      MOZ_ENABLE_WAYLAND        = "1";
      QT_QPA_PLATFORM           = "wayland";
      SDL_VIDEODRIVER           = "wayland";
    };
  };
}
