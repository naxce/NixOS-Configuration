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

  nativeBuildInputs = with pkgs; [ pkg-config ];
  buildInputs = with pkgs; [
    ncurses
    fftw
    libpulseaudio
  ];

  buildPhase = ''
    make
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp build/vis $out/bin/vis-visualizer
  '';
}
