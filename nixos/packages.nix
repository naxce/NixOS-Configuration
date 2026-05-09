{ config, pkgs, ... }:

{
  targets.genericLinux.enable = true;

  xdg.enable = true;
  xdg.mime.enable = true;
  xdg.systemDirs.data = [
    "${config.home.homeDirectory}/.nix-profile/share"
    "/var/lib/flatpak/exports/share"
  ];

  home.packages = with pkgs; [
    (stdenv.mkDerivation {
      name = "sddm-theme-forest";
      src = fetchFromGitHub {
        owner = "Darkkal44";
        repo = "qylock";
        rev = "main";
        sha256 = "sha256-4O8U8Wq+RInuG9mZq9X5Fp9GzG2ZJz8B1V3S8J8z7kU=";
      };
      installPhase = ''
        mkdir -p $out/share/sddm/themes
        cp -r themes/forest $out/share/sddm/themes/forest
      '';
    })

    # System
    home-manager
    os-prober
    tesseract
    kdiskmark
    refind
    killall
    kdePackages.kate
    kdePackages.ktextaddons
    peazip
    p7zip
    unrar
    rustdesk
    btop
    libreoffice
    lsof
    haveged
    libnotify
    iw
    linux-wifi-hotspot
    #lutris
    wineWow64Packages.stable
    winetricks
    gnome-calculator
    maliit-keyboard
    maliit-framework
    haruna
    audacity
    kdePackages.kdenlive

    # Gaming
    heroic
    mangohud
    gamemode
    gamescope
    protonup-qt
    antimicrox
    evtest
    libinput
    dualsensectl

    # Komunikacja
    vesktop
    #ferdium

    # Multimedia
    obs-studio
    davinci-resolve
    gimp

    # Nix Language
    vscodium
    nixfmt
    git

    # Kodowanie
    elmPackages.nodejs
    rustc
    cargo
    gcc
    pkg-config
    openssl
    qbittorrent
    filezilla
    github-desktop
    ollama

    (python311.withPackages (
      ps: with ps; [
        numpy
        pyaudio
        scipy
        colorama
      ]
    ))
    portaudio

    # Audio i Bluetooth
    cider-2
    pavucontrol
    bluez
    bluez-tools
    icloudpd

    # Sieć
    wireless-regdb

    # Ricing
    kitty
    fastfetch
    imagemagick
    cmatrix
    ascii-image-converter
    display3d
    asciiquarium-transparent
    btop
    lm_sensors
    sptlrx
    playerctl
    pipes-rs
    cbonsai
    tty-clock
    cava
    lavat
    toilet
  ];
}
