{
  lib,
  stdenv,
  meson,
  ninja,
  pkg-config,
  wayland,
  wayland-protocols,
  wlroots_0_20,
  cairo,
  pango,
  libxkbcommon,
  dbus,
  makeWrapper,
}:

stdenv.mkDerivation {
  pname = "zenithde";
  version = "1.0.0";

  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    makeWrapper
  ];

  buildInputs = [
    wayland
    wayland-protocols
    wlroots_0_20
    cairo
    pango
    libxkbcommon
    dbus
  ];

  preConfigure = ''
    export PKG_CONFIG_PATH="${wlroots_0_20}/lib/pkgconfig:${wayland}/lib/pkgconfig:${wayland-protocols}/share/pkgconfig:${cairo}/lib/pkgconfig:${pango}/lib/pkgconfig:${libxkbcommon}/lib/pkgconfig:${dbus}/lib/pkgconfig"
  '';

  postInstall = ''
    mkdir -p $out/share/wayland-sessions
    cp zenith.desktop $out/share/wayland-sessions/
  '';

  meta = with lib; {
    description = "Minimal gaming Wayland desktop environment";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "zenithde";
  };
}
