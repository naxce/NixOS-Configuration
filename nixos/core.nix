# core.nix

{
  config ? { },
  pkgs ? import <nixpkgs> { },
  lib ? pkgs.lib,
  ...
}:

{
  # Najnowszy kernel CachyOS
  boot.kernelPackages = pkgs.linuxPackages_xanmod;

  # Ustawienia konsoli
  console.font = "${pkgs.terminus_font}/share/consolefonts/ter-v24n.psf.gz";
  console.keyMap = "pl2";
  console.packages = [ pkgs.terminus_font ];

  # Bootloader
  boot.loader.efi.canTouchEfiVariables = true;
  boot.loader.refind.enable = false;
  boot.loader.systemd-boot.enable = true;
  boot.loader.systemd-boot.configurationLimit = 10;
  boot.loader.grub.enable = false;

  # Wirtualka
  virtualisation.libvirtd.enable = true;
  programs.virt-manager.enable = true;

  # Montowanie partycji Windowsa i Shared
  boot.supportedFilesystems = [ "ntfs" ];
  fileSystems."/mnt/windows" = {
    device = "/dev/disk/by-uuid/F898E05298E010C4";
    fsType = "ntfs-3g";
    options = [
      "rw"
      "uid=1000"
      "nofail"
    ];
  };
  fileSystems."/mnt/shared" = {
    device = "/dev/disk/by-uuid/19BF10154E8D57D3";
    fsType = "ntfs-3g";
    options = [
      "rw"
      "uid=1000"
      "gid=100"
      "umask=002"
      "nofail"
    ];
  };

  # Auto-sprzątanie
  nix.gc = {
    automatic = true;
    dates = "weekly";
    options = "--delete-older-than 7d";
  };
  nix.settings.auto-optimise-store = true;

  # Klawiatura ekranowa
  environment.variables = {
    KWIN_IM_SHOW_ALWAYS = "1";
    QT_IM_MODULE = "maliit";
  };

  # Kontroler
  boot.kernelModules = [
    "hid-sony"
    "hid-playstation"
    "joydev"
    "uinput"
  ];

  boot.extraModprobeConfig = ''
    options hid-playstation
  '';

  # NVIDIA
  services.xserver.videoDrivers = [ "nvidia" ];
  hardware.nvidia = {
    modesetting.enable = true;
    open = true;
    package = config.boot.kernelPackages.nvidiaPackages.stable;
  };

  hardware.graphics = {
    enable = true;
    enable32Bit = true;
  };
  hardware.enableRedistributableFirmware = true;

  programs.sway.enable = true;

  systemd.services.gpu-performance = {
    wantedBy = [ "multi-user.target" ];
    serviceConfig = {
      Type = "oneshot";
      ExecStart = ''
        ${config.hardware.nvidia.package.bin}/bin/nvidia-smi -pm 1
        ${config.hardware.nvidia.package.bin}/bin/nvidia-smi -lgc 300,2700
      '';
    };
  };

  # Parametry kernela
  boot.kernelParams = [
    "nvidia-drm.modeset=1"
    "nvidia-drm.fbdev=1"
    "split_lock_detect=off"
  ];

  # POLSKA 🇵🇱
  time.timeZone = "Europe/Warsaw";
  i18n.defaultLocale = "en_US.UTF-8";

  i18n.extraLocaleSettings = {
    LC_ADDRESS = "pl_PL.UTF-8";
    LC_IDENTIFICATION = "pl_PL.UTF-8";
    LC_MEASUREMENT = "pl_PL.UTF-8";
    LC_MONETARY = "pl_PL.UTF-8";
    LC_NAME = "pl_PL.UTF-8";
    LC_NUMERIC = "pl_PL.UTF-8";
    LC_PAPER = "pl_PL.UTF-8";
    LC_TELEPHONE = "pl_PL.UTF-8";
    LC_TIME = "pl_PL.UTF-8";
  };

  # Rebuild bez hasła
  security.sudo.extraRules = [
    {
      users = [ "naxce" ];
      commands = [
        {
          command = "/run/current-system/sw/bin/nixos-rebuild";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/nix-collect-garbage";

          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/nix-shell";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/create_ap";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/home/naxce/dotfiles/scripts/winboot.sh";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/tee /boot/EFI/refind/manual_boot.conf";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/systemctl reboot";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/home/naxce/dotfiles/scripts/hotspot.sh";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/create_ap";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/pkill";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/ip";
          options = [ "NOPASSWD" ];
        }
        {
          command = "/run/current-system/sw/bin/iw";
          options = [ "NOPASSWD" ];
        }
      ];
    }
  ];
  security.polkit.enable = true;

  security.polkit.extraConfig = ''
    polkit.addRule(function(action, subject) {
      if (subject.user == "naxce") {
        if (action.id == "org.freedesktop.policykit.exec" || 
            action.id == "org.freedesktop.login1.reboot" ||
            action.id == "org.blueman.network.setup" ||
            action.id == "org.blueman.rfkill.setstate" ||
            action.id == "org.blueman.pincode.confirm" ||
            action.id == "org.blueman.device.pair" ||
            action.id == "org.blueman.device.foregroundpair" ||
            action.id == "org.blueman.device.connect" ||
            action.id == "org.freedesktop.systemd1.manage-units") {
          return polkit.Result.YES;
        }
      }
    });
  '';

  systemd.services.reset-refind-config = {
    description = "rEFInd (NixOS)";
    wantedBy = [ "multi-user.target" ];
    serviceConfig = {
      Type = "oneshot";
      ExecStart = "${pkgs.bash}/bin/bash -c '${pkgs.coreutils}/bin/printf \"timeout 5\\ndefault_selection \\\"NixOS\\\"\\n\" > /boot/EFI/refind/manual_boot.conf'";
    };
  };

  # Bluetooth
  hardware.bluetooth = {
    enable = true;
    powerOnBoot = true;
    settings = {
      General = {
        Enable = "Source,Sink,Media,Socket";
        Experimental = true;
        FastConnectable = true;
      };
    };
  };
  services.blueman.enable = true;

  # Java
  programs.java.enable = true;

  #
  xdg.portal.enable = true;
  xdg.portal.extraPortals = with pkgs; [
    kdePackages.xdg-desktop-portal-kde
  ];

  environment.sessionVariables = {
    TESSDATA_PREFIX = "/run/current-system/sw/share/tessdata";
  };

  # PipeWire
  security.rtkit.enable = true;
  services.pipewire = {
    enable = true;
    alsa.enable = true;
    alsa.support32Bit = true;
    pulse.enable = true;
  };

  services.xserver.xkb.layout = "pl";

  # OpenRGB
  services.hardware.openrgb = {
    enable = true;
    motherboard = "amd";
  };
  systemd.user.services.openrgb-autostart = {
    description = "openrgb-autostart";
    after = [ "graphical-session.target" ];
    wantedBy = [ "graphical-session.target" ];
    serviceConfig = {
      ExecStart = "${pkgs.openrgb}/bin/openrgb --profile '1'";
      Type = "oneshot";
      RemainAfterExit = "yes";
    };
  };

  # Flatpak
  services.flatpak.enable = true;
}
