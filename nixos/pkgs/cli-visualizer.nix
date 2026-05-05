{
  pkgs ? import <nixpkgs> { },
}:

pkgs.stdenv.mkDerivation {
  pname = "cli-visualizer";
  version = "1.6";

  src = pkgs.fetchFromGitHub {
    owner = "PosixAlchemist";
    repo = "cli-visualizer";
    rev = "8a1317d7d36a56b9bd86506dfe9d30f63e81bb77";
    sha256 = "sha256-DurKc9g9SQT7AJSYoJsM5eWphPZLvkJ7Gg4KsXIdQl4=";
  };

  nativeBuildInputs = with pkgs; [
    pkg-config
    which
    gcc
  ];

  buildInputs = with pkgs; [
    ncurses
    fftw
    libpulseaudio
  ];

  buildPhase = ''
    mkdir -p build
    g++ -O3 -lpthread -lncurses -lfftw3 -lpulse -lrt \
        src/*.cpp src/Visualizers/*.cpp src/PostProcessors/*.cpp src/Transformers/*.cpp \
        -I./include -o build/vis-visualizer
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp build/vis-visualizer $out/bin/vis-visualizer
  '';
}
