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
        nixos: Rebuild with Flakes + Commit to GitHub
        nixgit: Commit to GitHub
        nixbuild: Rebuild with Flakes
        nixhome: Rebuild Home Manager Config
        nixplasma: Restart KDE Plasma
        nixclean: Collect and delete garbage
        rice: Show ricing style choice
        boot-windows: Immediately boot to Windows 11
        "
      '';
      nixos = "cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main && sudo nixos-rebuild switch --flake .#naxce";
      nixgit = "cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main";
      nixbuild = "cd ~/dotfiles && sudo nixos-rebuild switch --flake .#naxce";
      nixhome = "home-manager switch --flake ~/dotfiles#naxce";
      nixplasma = "plasmashell --replace & disown";
      nixclean = "sudo nix-collect-garbage -d && nix-collect-garbage -d";
      rice = "~/dotfiles/rice.sh";
      boot-windows = "sudo bootctl set-oneshot auto-windows && sudo bootctl set-timeout 1 && systemctl reboot";
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
