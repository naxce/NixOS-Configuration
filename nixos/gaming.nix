# gaming.nix

{ config, pkgs, ... }:

{
  # Steam
  programs.steam = {
    enable = true;
    protontricks.enable = true;
    remotePlay.openFirewall = true;
    dedicatedServer.openFirewall = true;
    gamescopeSession.enable = true;
  };

  # Gamemode
programs.gamemode = {
    enable = true;
    settings = {
      general = {
        softrealtime = "auto";
        renice = 10;
      };
      custom = {
        start = "${pkgs.libnotify}/bin/notify-send 'GameMode Started'";
        end = "${pkgs.libnotify}/bin/notify-send 'GameMode Ended'";
      };
    };
  };
  
  # Gamescope
  programs.gamescope = {
    enable = true;
    capSysNice = true;
  };

  environment.variables = {
    "__NV_PRIME_RENDER_OFFLOAD" = "1";
    "__GLX_VENDOR_LIBRARY_NAME" = "nvidia";
  };

  # Tryb wydajności
  powerManagement.cpuFreqGovernor = "performance";

  # Ustawienia kernela pod wydajność
  boot.kernel.sysctl = {
    "vm.max_map_count" = 2147483642;
    "fs.file-max" = 524288;
    "vm.swappiness" = 10;
    "kernel.sched_autogroup_enabled" = 1;
    "kernel.sched_migration_cost_ns" = 5000000;
    "net.core.default_qdisc" = "fq";
    "net.ipv4.tcp_congestion_control" = "bbr";
  };
}
