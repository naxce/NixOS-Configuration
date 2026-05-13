# packages.nix
{
  config,
  pkgs,
  zenith,
  ...
}:

{
  targets.genericLinux.enable = true;

  xdg.enable = true;
  xdg.mime.enable = true;

  xdg.systemDirs.data = [
    "${config.home.homeDirectory}/.nix-profile/share"
    "/var/lib/flatpak/exports/share"
  ];

  home.packages = with pkgs; [
    # FATUM
    zenith.defaultPackage.${pkgs.system}

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
    wdisplays
    wlr-randr
    cage

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
    htop
    btop
    fastfetch
    lm_sensors
    efibootmgr
    nvtopPackages.full

    # KDE
    kdePackages.kate
    kdePackages.ktextaddons
    kdePackages.kfind

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
    vulkan-tools

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

    clang-tools
    cmake

    rustc
    cargo
    cpio
    squashfsTools
    gnumake
    pkgsStatic.busybox
    curl

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
    deskreen

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
    inetutils
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
