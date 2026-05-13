/*
 * nix/module.nix — NixOS module for ZenithWM
 *
 * Usage in configuration.nix:
 *
 *   let zenithwm = import /path/to/ZenithWM/nix/default.nix { inherit pkgs; };
 *   in {
 *     imports = [ /path/to/ZenithWM/nix/module.nix ];
 *     services.zenithwm.enable = true;
 *     services.zenithwm.nvidia = true;   # set if you use NVIDIA
 *   }
 */

{ config, lib, pkgs, ... }:

let
  cfg = config.services.zenithwm;
  zenithwm = import ./default.nix { inherit pkgs; };
in {

  options.services.zenithwm = {
    enable = lib.mkEnableOption "ZenithWM Wayland compositor";

    nvidia = lib.mkOption {
      type    = lib.types.bool;
      default = false;
      description = "Enable NVIDIA-specific environment variables and kernel params.";
    };

    extraSessionVars = lib.mkOption {
      type    = lib.types.attrsOf lib.types.str;
      default = {};
      description = "Extra environment variables added to the session.";
    };
  };

  config = lib.mkIf cfg.enable {

    # ── Package ──────────────────────────────────────────────────────
    environment.systemPackages = [
      zenithwm
      pkgs.foot          # default terminal
      pkgs.fuzzel        # Super+r launcher (optional)
      pkgs.xwayland      # legacy game support
    ];

    # ── SDDM ─────────────────────────────────────────────────────────
    services.displayManager.sddm = {
      enable      = lib.mkDefault true;
      wayland.enable = true;
    };

    # ── Wayland / XDG ────────────────────────────────────────────────
    xdg.portal = {
      enable = true;
      extraPortals = [ pkgs.xdg-desktop-portal-wlr ];
    };

    # ── Session environment vars ──────────────────────────────────────
    environment.sessionVariables = lib.mkMerge [
      {
        XDG_SESSION_TYPE        = "wayland";
        XDG_CURRENT_DESKTOP     = "ZenithWM";
        MOZ_ENABLE_WAYLAND      = "1";
        QT_QPA_PLATFORM         = "wayland";
        SDL_VIDEODRIVER         = "wayland";
        GDK_BACKEND             = "wayland";
        CLUTTER_BACKEND         = "wayland";
      }
      (lib.mkIf cfg.nvidia {
        GBM_BACKEND                 = "nvidia-drm";
        __GLX_VENDOR_LIBRARY_NAME   = "nvidia";
        WLR_NO_HARDWARE_CURSORS     = "1";
        WLR_RENDERER                = "vulkan";
        LIBVA_DRIVER_NAME           = "nvidia";
        NVD_BACKEND                 = "direct";
      })
      cfg.extraSessionVars
    ];

    # ── NVIDIA kernel & driver settings ──────────────────────────────
    hardware = lib.mkIf cfg.nvidia {
      nvidia = {
        modesetting.enable = true;       # required for Wayland
        powerManagement.enable = false;  # disable for desktop/gaming
        open           = false;          # use proprietary driver
        nvidiaSettings = true;
      };
      graphics = {
        enable      = true;
        enable32Bit = true;              # 32-bit Steam / Proton
        extraPackages = with pkgs; [
          nvidia-vaapi-driver
          vaapiVdpau
          libvdpau-va-gl
        ];
      };
    };

    # ── Boot params for NVIDIA DRM ────────────────────────────────────
    boot = lib.mkIf cfg.nvidia {
      kernelParams = [ "nvidia-drm.modeset=1" "nvidia-drm.fbdev=1" ];
      kernelModules = [ "nvidia" "nvidia_modeset" "nvidia_uvm" "nvidia_drm" ];
    };

    # ── Audio (Pipewire) ─────────────────────────────────────────────
    services.pipewire = {
      enable            = lib.mkDefault true;
      alsa.enable       = true;
      alsa.support32Bit = true;
      pulse.enable      = true;
    };
    security.rtkit.enable = lib.mkDefault true;

    # ── D-Bus ────────────────────────────────────────────────────────
    services.dbus.enable = true;

    # ── GameMode ─────────────────────────────────────────────────────
    programs.gamemode.enable = lib.mkDefault true;

    # ── Polkit (needed for shutdown/reboot from taskbar) ─────────────
    security.polkit.enable = true;

  };
}
