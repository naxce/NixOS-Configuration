{ config, pkgs, ... }:

{
  home.username = "naxce";
  home.homeDirectory = "/home/naxce";
  home.stateVersion = "26.05";
  home.preferXdgDirectories = true;

  programs.home-manager.enable = true;

  home.file.".config/cava".source = ./.config/cava;
  home.file.".config/fastfetch".source = ./.config/fastfetch;
  home.file.".config/kitty".source = ./.config/kitty;
  home.file.".config/sptlrx".source = ./.config/sptlrx;

  programs.bash = {
    enable = true;
    shellAliases = {
      nixhelp = ''
        echo "
        nixos: Update + Rebuild + Git Push
        nixgit: Commit to GitHub
        nixbuild: Rebuild with Flakes
        nixhome: Rebuild Home Manager Config
        nixplasma: Restart KDE Plasma
        nixclean: Collect and delete garbage
        nixsh: nix-shell
        rice: Show ricing style choice
        "
      '';
      nixos = "clear && cd ~/dotfiles && nix flake update && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main && sudo nixos-rebuild switch --flake .#naxce";
      nixgit = "clear && cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main";
      nixbuild = "clear && cd ~/dotfiles && sudo nixos-rebuild switch --flake .#naxce";
      nixhome = "clear && home-manager switch --flake ~/dotfiles#naxce";
      nixplasma = "clear && plasmashell --replace & disown";
      nixclean = "clear && sudo nix-collect-garbage -d && nix-collect-garbage -d";
      nixsh = "clear && nix-shell";
      rice = "clear && ~/dotfiles/scripts/rice.sh";
    };
  };

  programs.git = {
    enable = true;
    settings.user.name = "naxce";
    settings.user.email = "naxcehelltea@gmail.com";
  };

  dconf.settings = {
    "org/gnome/desktop/interface" = {
      color-scheme = "prefer-dark";
    };
  };
}
