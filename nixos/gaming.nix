# gaming.nix

{ config, pkgs, ... }:

{
  # Steam
  programs.steam = {
    enable = true;
    remotePlay.openFirewall = true;
    dedicatedServer.openFirewall = true;
    gamescopeSession.enable = true;
  };

  # Gamemode
  programs.gamemode = {
    enable = true;
    settings = {
      general = {
        renice = 10;
      };
      gpu = {
        apply_gpu_optimisations = "accept-responsibility";
        gpu_device = 0;
        nv_powermode_rw = 1;
      };
    };
  };

  # Gamescope
  programs.gamescope = {
    enable = true;
    capSysNice = true;
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
