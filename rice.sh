#!/usr/bin/env bash

pkill -f kitty

CHOICE=$(zenity --list \
    --title="Ricing Selector" \
    --column="Theme" \
    --text="Choose the style:" \
    "Blue" \
    "Red" \
    "Purple" \
    " " \
    "Change Wallpaper" \
    --width=300 --height=450)

case "$CHOICE" in
    Blue)
        ~/dotfiles/blue.sh
        ;;
    Red)
        ~/dotfiles/red.sh
        ;;
    Purple)
        ~/dotfiles/purple.sh
        ;;
    "Change Wallpaper")
        ~/dotfiles/wallpaper.sh
        ;;
esac