{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs)
    lib
    stdenv
    meson
    ninja
    pkg-config
    wlroots_0_18
    wayland
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
  ];

  buildInputs = [
    wlroots_0_18
    wayland
    wayland-protocols
    libxkbcommon
    pixman
    xwayland

    gtk4
    gtk4-layer-shell
  ];

  mesonFlags = [
    "--buildtype=release"
    "-Db_lto=true"
  ];

  # ważne dla wlroots unstable API
  NIX_CFLAGS_COMPILE = "-DWLR_USE_UNSTABLE";

  # ✔ NIE używamy dwóch phase na to samo
  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    mkdir -p $out/share/wayland-sessions
    mkdir -p $out/share/zenithwm/defaults

    # binary
    if [ -f build/zenithwm ]; then
      install -Dm755 build/zenithwm $out/bin/zenithwm
    fi

    # session
    install -Dm644 session/zenithwm.desktop \
      $out/share/wayland-sessions/zenithwm.desktop

    # configs
    install -Dm644 config/zenithwm.conf \
      $out/share/zenithwm/defaults/zenithwm.conf

    install -Dm644 config/keys.conf \
      $out/share/zenithwm/defaults/keys.conf

    install -Dm644 config/desktop.conf \
      $out/share/zenithwm/defaults/desktop.conf

    # launcher script
    install -Dm755 session/zenithwm-session \
      $out/bin/zenithwm-session

    runHook postInstall
  '';

  meta = with lib; {
    description = "Minimal wlroots Wayland compositor optimised for gaming";
    homepage = "https://github.com/youruser/zenithwm";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [];
  };
}