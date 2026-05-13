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

  mesonFlags = [
    "--buildtype=release"
    "-Db_lto=true"
  ];

  NIX_CFLAGS_COMPILE = [
    "-DWLR_USE_UNSTABLE"
    "-I${wayland-protocols}/share/wayland-protocols/stable/xdg-shell"
    "-I${wayland}/include"
  ];

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

  passthru = {
    providedSessions = [ "zenithwm" ];
  };

  meta = with lib; {
    description = "Minimal wlroots Wayland compositor optimised for gaming";
    homepage = "https://github.com/youruser/zenithwm";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [];
  };
}