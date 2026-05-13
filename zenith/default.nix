{ lib, stdenv, meson, ninja, pkg-config, wayland, wayland-protocols, wlroots,
  cairo, pango, libxkbcommon, dbus, makeWrapper }:

stdenv.mkDerivation {
  pname = "zenith";
  version = "1.0.0";

  src = ./.;

  nativeBuildInputs = [ meson ninja pkg-config makeWrapper ];
  buildInputs = [ wayland wayland-protocols wlroots cairo pango libxkbcommon dbus ];

  mesonFlags = [ "-Dbuildtype=release" ];

  postInstall = ''
    mkdir -p $out/share/wayland-sessions
    cp zenith.desktop $out/share/wayland-sessions/
  '';

  meta = with lib; {
    description = "Minimal gaming Wayland desktop environment";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "zenith";
  };
}