self:
{
  config,
  lib,
  pkgs,
  ...
}:

let
  cfg = config.services.zenithde;
in
{
  options.services.zenithde = {
    enable = lib.mkEnableOption "ZenithDE desktop environment session";
  };

  config = lib.mkIf cfg.enable {
    home.packages = [ self.packages.${pkgs.system}.zenithde ];
    xdg.dataFile."wayland-sessions/zenithde.desktop".source = "${
      self.packages.${pkgs.system}.zenithde
    }/share/wayland-sessions/zenithde.desktop";
  };
}
