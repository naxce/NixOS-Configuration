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
      sh -c "tput civis; ~/dotfiles/.config/kitty/weather.sh; tail -f /dev/null" &

# 7. Stats
kitty --title "stats" $FLAGS \
  sh -c "~/.config/kitty/sys_info.sh" &

# 8. I <3 JULKA
kitty --title "love" $FLAGS \
      --override window_padding_width=0 \
      --override window_margin_width=0 \
      sh -c "
        tput civis; 
        clear;
        
        echo -e '\033[1;34m';

        tput cup 4 0;
        toilet -f mono9 \"  I\" | sed 's/^/              /'
        
        tput cup 12 0;
        toilet -f mono9 \" <3\" | sed 's/^/              /'
        
        tput cup 20 0;
        toilet -f mono9 \"JULKA\" | sed 's/^/              /'

        tail -f /dev/null
      " &

# 9. Cava
kitty --title "visualizer" $FLAGS \
      sh -c "cava || tail -f /dev/null" &

# 10. CBonsai
kitty --title "cbonsai" $FLAGS \
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
  sh -c "cbonsai -l -b 1 || tail -f /dev/null" &

# 11. Lavat
kitty --title "lavat" $FLAGS \
  --override window_padding_width=0 \
  sh -c "lavat -c blue -r 1; tail -f /dev/null" &

# 12. Asciiquarium
kitty --title "asciiquarium" $FLAGS \
  --override font_size=6 \
  --override window_padding_width=0 \
  --override placement_strategy=center \
  sh -c "asciiquarium -t || tail -f /dev/null" &

# 13. Lyrics
kitty --title "lyrics" $FLAGS \
  sptlrx &

# 14. Pipes
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