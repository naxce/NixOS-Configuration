{ pkgs ? import <nixpkgs> {} }:

let
  patched_wlroots = pkgs.wlroots_0_18.overrideAttrs (old: {
    pname = "wlroots-zenith-fixed";
    version = "${old.version}-patched";
    __intentionallyOverridingVersion = true; 
    
    postPatch = (old.postPatch or "") + ''
      find . -name "meson.build" -exec sed -i 's/-Werror//g' {} +
      if [ -f backend/libinput/switch.c ]; then
        sed -i '/case LIBINPUT_SWITCH_TABLET_MODE:/a \                case LIBINPUT_SWITCH_KEYPAD_SLIDE: break;' backend/libinput/switch.c
      fi
    '';
  });

  inherit (pkgs)
    lib
    stdenv
    meson
    ninja
    pkg-config
    wayland
    wayland-scanner
    wayland-protocols
    libxkbcommon
    pixman
    gtk4
    gtk4-layer-shell
    xwayland;
in

stdenv.mkDerivation {
  pname = "zenithwm";
  version = "0.1.0";

  src = ../.;

  strictDeps = true;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    wayland-scanner
  ];

  buildInputs = [
    patched_wlroots
    wayland
    wayland-protocols
    libxkbcommon
    pixman
    xwayland
    gtk4
    gtk4-layer-shell
  ];

  preBuild = ''
    mkdir -p protocols
    wayland-scanner client-header \
      ${wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml \
      protocols/xdg-shell-protocol.h
    wayland-scanner private-code \
      ${wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml \
      protocols/xdg-shell-protocol.c
  '';

  NIX_CFLAGS_COMPILE = [
    "-DWLR_USE_UNSTABLE"
    "-I./protocols"
  ];

  mesonFlags = [
    "--buildtype=release"
    "-Db_lto=true"
  ];

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    mkdir -p $out/share/wayland-sessions
    mkdir -p $out/share/zenithwm/defaults

    # Szukamy binarki w build/ lub bezpośrednio w build/compositor/
    if [ -f build/zenithwm ]; then
      install -Dm755 build/zenithwm $out/bin/zenithwm
    elif [ -f build/compositor/zenithwm ]; then
      install -Dm755 build/compositor/zenithwm $out/bin/zenithwm
    fi

    # session
    if [ -f session/zenithwm.desktop ]; then
      install -Dm644 session/zenithwm.desktop $out/share/wayland-sessions/zenithwm.desktop
    fi

    # configs
    [ -d config ] && cp -r config/* $out/share/zenithwm/defaults/

    # launcher script
    if [ -f session/zenithwm-session ]; then
      install -Dm755 session/zenithwm-session $out/bin/zenithwm-session
    fi

    runHook postInstall
  '';

  passthru = {
    providedSessions = [ "zenithwm" ];
  };

  meta = with lib; {
    description = "Minimal wlroots Wayland compositor optimised for gaming";
    homepage = "https://github.com/naxce/ZenithWM";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [];
  };
}