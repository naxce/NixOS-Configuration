{
  config,
  pkgs,
  ...
}:

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
        reset &&
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
      nixos = "reset && cd ~/dotfiles && nix flake update && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main && sudo nixos-rebuild switch --flake .#naxce";
      nixgit = "reset && cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main";
      nixbuild = "reset && cd ~/dotfiles && sudo nixos-rebuild switch --flake .#naxce";
      nixhome = "reset && home-manager switch --flake ~/dotfiles#naxce";
      nixplasma = "reset && plasmashell --replace & disown";
      nixclean = "reset && sudo nix-collect-garbage -d && nix-collect-garbage -d";
      nixsh = "reset && nix-shell";
      zenithgit = "reset && cd ~/dotfiles/zenith && git add . && (git commit -m \"Update\" || true) && git push origin main";
      rice = "reset && ~/dotfiles/scripts/rice.sh";
      khelp = ''
        reset &&
        echo "
        ===============================
                KITTY WORK HELP
        ===============================

        TABS
        Ctrl+Shift+T   → new tab
        Ctrl+Shift+W   → close tab
        Ctrl+Shift+Q   → close window

        SPLITS
        Ctrl+Shift+Enter → split window
        Ctrl+Alt+V       → split
        Ctrl+Alt+H       → split

        NAVIGATION
        Ctrl+Alt+arrows → move between panes

        RESIZE
        Ctrl+Shift+arrows → resize split

        ===============================
        TIP
        splits auto-arrange (no manual direction)
        ===============================
        "
      '';
      ff = ''
        mkdir -p /tmp/kittywork

        cat << 'EOF' > /tmp/kittywork/fastfetch.jsonc
        {
            "$schema": "https://github.com/fastfetch-cli/fastfetch/raw/dev/doc/json_schema.json",
            "logo": {
                "source": "~/dotfiles/Pictures/LogoBlue.png",
                "type": "kitty",
                "width": 25,
                "height": 10,
                "padding": { "top": 2, "left": 2 }
            },
            "display": {
                "separator": " ➜ ",
                "color": { "keys": "blue" }
            },
            "modules": [
                "title",
                "separator",
                { "type": "os", "key": "󱄅", "format": "{2} {8}" },
                { "type": "kernel", "key": "󰌽", "format": "{2}" },
                { "type": "uptime", "key": "󱎫" },
                { "type": "shell", "key": "󱆃" },
                { "type": "cpu", "key": "󰻠", "format": "{1}" },
                { "type": "gpu", "key": "󰢮", "hideType": "integrated", "format": "{2}" },
                { 
                    "type": "display", 
                    "key": "󰍹", 
                    "compactType": "original-with-refresh",
                    "format": "{1}x{2} @ {3}Hz" 
                },
                { "type": "memory", "key": "󰑭" },
                { "type": "localip", "key": "󰩟", "showIpv6": false }
            ]
        }
        EOF


        reset && fastfetch --config /tmp/kittywork/fastfetch.jsonc
      '';
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

  /*
    home.packages = with pkgs; [
      zenith.packages.${pkgs.system}.zenith
    ];
  */
}
