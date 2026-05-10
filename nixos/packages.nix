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
    # SYSTEM
    home-manager
    os-prober
    kdiskmark
    refind
    killall
    lsof
    haveged
    libnotify
    iw
    linux-wifi-hotspot

    # Fonts
    inter
    jetbrains-mono
    nerd-fonts.jetbrains-mono
    nerd-fonts.symbols-only
    noto-fonts
    noto-fonts-color-emoji

    # Archives
    peazip
    p7zip
    unrar

    # Terminal
    btop
    fastfetch
    lm_sensors

    # KDE
    kdePackages.kate
    kdePackages.ktextaddons

    # Audio / multimedia tools
    haruna
    audacity
    gnome-calculator
    maliit-keyboard
    maliit-framework

    # Office
    libreoffice

    # GAMING
    heroic
    prismlauncher
    mangohud
    gamemode
    gamescope
    protonup-qt
    antimicrox
    evtest
    libinput
    dualsensectl

    wineWow64Packages.stable
    winetricks

    # COMMUNICATION
    vesktop
    # ferdium

    # MEDIA
    obs-studio
    gimp
    davinci-resolve
    kdePackages.kdenlive

    # DEV
    vscodium
    nixfmt
    git

    gcc
    pkg-config
    openssl

    rustc
    cargo

    elmPackages.nodejs

    (python311.withPackages (
      ps: with ps; [
        numpy
        scipy
        pyaudio
        colorama
      ]
    ))

    portaudio

    # TOOLS
    qbittorrent
    filezilla
    github-desktop
    ollama
    rustdesk
    anydesk

    # AUDIO / BLUETOOTH
    pavucontrol
    bluez
    bluez-tools
    cider-2
    icloudpd

    # NETWORK
    wireless-regdb

    # RICING
    zenity
    kitty
    imagemagick
    cmatrix
    ascii-image-converter
    display3d
    asciiquarium-transparent
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
