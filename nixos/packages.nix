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
      pname = "zenithwm";
      version = "1.0";
      src = ./ZenithWM;
      buildInputs = [ 
        raylib 
        wlroots_0_20
        wayland 
        libxkbcommon 
        pixman 
      ];
      nativeBuildInputs = [ pkg-config gcc ];
      unpackPhase = "true";
      buildPhase = ''
        g++ -o zenith ${./ZenithWM/zenith.cpp} \
          -lraylib -lwayland-server -lrt -lm -ldl -lpthread
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp zenith $out/bin/
      '';
    })
    # ZENITH
    raylib 
    wayland 
    wayland-protocols
    libxkbcommon
    libGL
    xorg.libX11
    xorg.libXcursor
    xorg.libXrandr
    xorg.libXi
    xorg.libXinerama
    
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
