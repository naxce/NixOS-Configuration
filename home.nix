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

  home.packages = [
    (pkgs.writeShellScriptBin "kwork" ''
      exec kitty --class kitty-work --name kitty-work --config '/home/naxce/dotfiles/.config/kitty/work.conf' "$@"
    '')
  ];

  home.file.".config/cava".source = ./.config/cava;
  home.file.".config/fastfetch".source = ./.config/fastfetch;
  home.file.".config/kitty".source = ./.config/kitty;
  home.file.".config/sptlrx".source = ./.config/sptlrx;

  programs.bash = {
    enable = true;
    initExtra = ''
      mkdir -p /tmp/kittywork
      cat << 'EOF' > /tmp/kittywork/fastfetch.jsonc
      {
          "$schema": "https://github.com/fastfetch-cli/fastfetch/raw/dev/doc/json_schema.json",
          "logo": {
              "source": "~/dotfiles/Pictures/LogoBlue.png",
              "type": "kitty",
              "width": 26,
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

      wipe() {
          command reset
          fastfetch --config /tmp/kittywork/fastfetch.jsonc
      }

      fastfetch --config /tmp/kittywork/fastfetch.jsonc
    '';

    shellAliases = {
      ff = "fastfetch --config /tmp/kittywork/fastfetch.jsonc";
      wipe = "wipe";

      nixhelp = "wipe && echo -e \"\nnixos: Update + Rebuild + Git Push\nnixgit: Commit to GitHub\nnixbuild: Rebuild with Flakes\nnixhome: Rebuild Home Manager Config\nnixkde: Restart KDE Plasma\nnixclean: Collect and delete garbage\nnixsh: nix-shell\nrice: Show ricing style choice\n\"";

      nixos = "wipe && cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main && sudo nixos-rebuild switch --flake .#naxce";
      nixup = "wipe && cd ~/dotfiles && nix flake update && git add . && git commit -m \"Update $(date)\" || true && git push origin main && sudo nixos-rebuild switch --flake .#naxce";
      nixgit = "wipe && cd ~/dotfiles && git add . && (git commit -m \"Update $(date)\" || true) && git push origin main";
      nixbuild = "wipe && cd ~/dotfiles && sudo nixos-rebuild switch --flake .#naxce";
      nixhome = "wipe && home-manager switch --flake ~/dotfiles#naxce";
      nixkde = "wipe && { plasmashell --replace & disown; }";
      nixclean = "wipe && sudo nix-collect-garbage -d && nix-collect-garbage -d";
      nixsh = "wipe && nix-shell";
      hts = "sudo ~/dotfiles/scripts/hotspot.sh";
      zenithgit = "wipe && cd ~/dotfiles/zenith && git add . && (git commit -m \"Update\" || true) && git push origin main";
      rice = "wipe && ~/dotfiles/scripts/rice.sh";

      khelp = "wipe && echo -e \"\n===============================\n        KITTY WORK HELP\n===============================\n\nTABS\nCtrl+Shift+T   → new tab\nCtrl+Shift+W   → close tab\nCtrl+Shift+Q   → close window\n\nSPLITS\nCtrl+Shift+Enter → split window\nCtrl+Alt+V       → split\nCtrl+Alt+H       → split\n\nNAVIGATION\nCtrl+Alt+arrows → move between panes\n\nRESIZE\nCtrl+Shift+arrows → resize split\n\n===============================\nTIP\nsplits auto-arrange (no manual direction)\n===============================\n\"";
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
