{ config, pkgs, ... }:

{
  home.username = "naxce";
  home.homeDirectory = "/home/naxce";
  home.stateVersion = "26.05";

  programs.home-manager.enable = true;

  home.file.".config/cava".source = ./.config/cava;
  home.file.".config/fastfetch".source = ./.config/fastfetch;
  home.file.".config/kitty".source = ./.config/kitty;
  home.file.".config/sptlrx".source = ./.config/sptlrx;
  home.file.".config/vis".source = ./.config/vis;

  programs.bash = {
    enable = true;
    shellAliases = {
      nixos = "cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main --force && sudo nixos-rebuild switch --flake .#naxce";

      onlybuild = "cd ~/dotfiles && git add . && sudo nixos-rebuild switch --flake .#naxce";

      nixclean = "sudo nix-collect-garbage -d && nix-collect-garbage -d";
    };
  };

  programs.git = {
    enable = true;
    userName = "naxce";
    userEmail = "naxcehelltea@gmail.com";
  };
}
