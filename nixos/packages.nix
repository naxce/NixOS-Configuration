{ config, pkgs, ... }:

{
  environment.systemPackages = with pkgs; [
    # System
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
    brave
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

    # Audio i Bluetooth
    cider-2
    pavucontrol
    bluez
    bluez-tools

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
    toilet
    tty-clock
    cava
    cavalier
  ];
}
