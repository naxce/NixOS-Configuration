{
  lib,
  stdenv,
  cmake,
  pkg-config,
  wayland,
  wayland-scanner,
  wayland-protocols,
  libdrm,
  mesa,
  libGL,
  libgbm,
  libinput,
  libudev-zero,
  cairo,
  pixman,
  fontconfig,
  seatd,
}:

stdenv.mkDerivation {
  pname = "zenith";
  version = "1.0.0";

  src = ./.;

  nativeBuildInputs = [
    cmake
    pkg-config
    wayland-scanner
    wayland-protocols
  ];

  buildInputs = [
    wayland
    libdrm
    mesa
    libGL
    libgbm
    libinput
    libudev-zero
    cairo
    pixman
    fontconfig
    seatd
  ];

  preConfigure = ''
    mkdir -p protocol
    cp ${wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml protocol/xdg-shell.xml
  '';

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  postInstall = ''
    install -Dm644 $src/nixos/zenith.desktop $out/share/wayland-sessions/zenith.desktop
    install -Dm644 $src/nixos/zenith-sddm.desktop $out/share/xsessions/zenith.desktop
  '';

  passthru.providedSessions = [ "zenith" ];

  meta = with lib; {
    description = "Zenith minimal Wayland compositor optimized for gaming";
    homepage = "https://github.com/naxce/zenith";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "zenith";
  };
}
