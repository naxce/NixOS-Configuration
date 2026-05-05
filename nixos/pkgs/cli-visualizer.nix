{
  pkgs ? import <nixpkgs> { },
}:

pkgs.stdenv.mkDerivation {
  pname = "cli-visualizer";
  version = "1.6";

  src = pkgs.fetchFromGitHub {
    owner = "PosixAlchemist";
    repo = "cli-visualizer";
    rev = "v1.6";
    sha256 = pkgs.lib.fakeSha256;
  };

  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
  ];
  buildInputs = with pkgs; [
    ncurses
    fftw
    libpulseaudio
  ];

  cmakeFlags = [ "-DFFTW_INCLUDE_DIR=${pkgs.fftw.dev}/include" ];
}
