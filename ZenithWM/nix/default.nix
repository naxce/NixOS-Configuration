/*
 * nix/default.nix — ZenithWM package
 *
 * Build:  nix-build nix/default.nix
 * Shell:  nix-shell nix/default.nix
 */

{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs) lib stdenv meson ninja pkg-config
    wlroots_0_18 wayland wayland-protocols libxkbcommon pixman
    gtk4 gtk4-layer-shell wayland-utils xwayland;
in

stdenv.mkDerivation {
  pname = "zenithwm";
  version = "0.1.0";

  src = ../.;   # root of the repo

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];

  buildInputs = [
    # Compositor
    wlroots_0_18
    wayland
    wayland-protocols
    libxkbcommon
    pixman
    xwayland          # optional XWayland support

    # Taskbar + monitor tool
    gtk4
    gtk4-layer-shell
  ];

  mesonFlags = [
    "--buildtype=release"       # -O3 + LTO via meson default_options
    "-Db_lto=true"
  ];

  installPhase = ''
    mkdir -p $out/share/wayland-sessions
    cp session/zenithwm.desktop $out/share/wayland-sessions/
  '';

  # Let wlroots use unstable internal API
  NIX_CFLAGS_COMPILE = "-DWLR_USE_UNSTABLE";

  postInstall = ''
    # SDDM session entry
    install -Dm644 $src/sddm/zenithwm.desktop \
      $out/share/wayland-sessions/zenithwm.desktop

    # Default configs
    install -Dm644 $src/config/zenithwm.conf \
      $out/share/zenithwm/defaults/zenithwm.conf
    install -Dm644 $src/config/keys.conf \
      $out/share/zenithwm/defaults/keys.conf
    install -Dm644 $src/config/desktop.conf \
      $out/share/zenithwm/defaults/desktop.conf

    # Session script
    install -Dm755 $src/session/zenithwm-session \
      $out/bin/zenithwm-session
  '';

  meta = with lib; {
    description = "Minimal wlroots Wayland compositor optimised for gaming";
    homepage    = "https://github.com/youruser/zenithwm";
    license     = licenses.mit;
    platforms   = platforms.linux;
    maintainers = [];
  };
}
