#!/usr/bin/env bash

pkill -f kitty
rm -f /tmp/julka_rice.png
sleep 1

FLAGS="--override remember_window_size=no --override cursor_opacity=0 --override shell_integration=disabled"

# 1. FastFetch
kitty --title "fetch" $FLAGS \
      sh -c "tput civis; fastfetch; tail -f /dev/null" &

# 2. Display3D
kitty --title "display3d" $FLAGS \
      sh -c "
    display3d ~/NixOS/resources/blahaj.obj -t 0,0,4.5
  " &

# 3. Matrix
kitty --title "matrix" $FLAGS \
      sh -c "tput civis; cmatrix -C blue -s" &

# 4. Clock
kitty --title "clock" $FLAGS \
      sh -c "tty-clock -S -c -C 4 -b -B || tail -f /dev/null" &

# 5. Calendar
kitty --title "calendar" $FLAGS \
      sh -c "tput civis; echo -e '\n\n\033[1;34m'; cal; tail -f /dev/null" &

# 6. Weather
kitty --title "weather" $FLAGS \
      sh -c "tput civis; ~/.config/kitty/weather.sh; tail -f /dev/null" &

# 7. Stats
kitty --title "stats" $FLAGS \
  sh -c "~/.config/kitty/sys_info.sh" &

# 8. naxce
kitty --title "naxce" $FLAGS \
      --override window_padding_width=0 \
      --override window_margin_width=0 \
      sh -c "
        tput civis; 
        clear;
        while true; do
            tput cup 8 0;
            echo -e '\033[1;34m';
            nix-shell -p toilet --run 'toilet -f mono9 \"naxce\"' | sed 's/^/                /'
            tput el;
            sleep 60;
        done
      " &

# 9. Cava
kitty --title "visualizer" $FLAGS \
      sh -c "cava || tail -f /dev/null" &

# 10. Cavalier
kitty --title "cavalier" $FLAGS \
      --override window_padding_width=0 \
      --override window_margin_width=0 \
      sh -c "sleep 1; nix run nixpkgs#cavalier || tail -f /dev/null" &

# 12. Julka
kitty --title "julka" $FLAGS \
  --override window_padding_width=0 \
  --override background_image=~/NixOS/julcia.png \
  --override background_image_layout=scaled \
  --override background_image_linear=yes \
  sh -c "tput civis; clear; tail -f /dev/null" &

# 13. Asciiquarium
kitty --title "asciiquarium" $FLAGS \
  --override font_size=6 \
  --override window_padding_width=0 \
  --override placement_strategy=center \
  sh -c "asciiquarium -t || tail -f /dev/null" &

# 14. Lyrics
kitty --title "lyrics" $FLAGS \
  sptlrx &

# 15. Pipes
kitty --title "pipes" $FLAGS \
  --override color1=#89b4fa \
  --override color2=#74c7ec \
  --override color3=#94e2d5 \
  --override color4=#89b4fa \
  --override color5=#74c7ec \
  --override color6=#94e2d5 \
  --override color7=#bac2de \
  \
  --override color8=#585b70 \
  --override color9=#89b4fa \
  --override color10=#74c7ec \
  --override color11=#94e2d5 \
  --override color12=#89b4fa \
  --override color13=#74c7ec \
  --override color14=#94e2d5 \
  --override color15=#a6adc8 \
  \
  sh -c "pipes-rs -c ansi --palette default -k curved -b true || tail -f /dev/null" &

disown
exit 0