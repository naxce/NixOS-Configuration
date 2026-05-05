{ config, pkgs, ... }:

{
  # Użytkownik
  home.username = "naxce";
  home.homeDirectory = "/home/naxce";
  home.stateVersion = "26.05";

  # Home Manager
  programs.home-manager.enable = true;

  # Dotfiles
  home.file.".config/cava".source = ./.config/cava;
  home.file.".config/fastfetch".source = ./.config/fastfetch;
  home.file.".config/kitty".source = ./.config/kitty;
  home.file.".config/sptlrx".source = ./.config/sptlrx;

  # Alias "nixos"
  programs.bash = {
    enable = true;
    shellAliases = {
      nixos = "cd ~/dotfiles && git fetch origin main && git reset --soft origin/main && git add . && git rm --cached nixos/hardware-configuration.nix --ignore-unmatch && git add -f -N nixos/hardware-configuration.nix && (git commit -m \"Update $(date)\" || true) && git push origin main --force && sudo nixos-rebuild switch --flake .#naxce";
    };
  };

  # Git
  programs.git = {
    enable = true;
    settings.user.name = "naxce";
    settings.user.email = "naxcehelltea@gmail.com";
  };
}
