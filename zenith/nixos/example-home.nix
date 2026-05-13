{ config, pkgs, ... }:

{
  imports = [
    (builtins.getFlake "path:/path/to/zenith").homeManagerModules.default
  ];

  wayland.windowManager.zenith = {
    enable = true;
    settings = {
      terminal = "foot";
      gaps = 0;
      borderWidth = 1;
      nvidia = true;
    };
    autostart = [
      "foot --server"
    ];
  };
}
