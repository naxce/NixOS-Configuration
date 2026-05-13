self: { config, lib, pkgs, ... }:

let
  cfg = config.services.zenith;
in {
  options.services.zenith = {
    enable = lib.mkEnableOption "Zenith desktop environment session";
  };

  config = lib.mkIf cfg.enable {
    home.packages = [ self.packages.${pkgs.system}.zenith ];
    xdg.dataFile."wayland-sessions/zenith.desktop".source =
      "${self.packages.${pkgs.system}.zenith}/share/wayland-sessions/zenith.desktop";
  };
}