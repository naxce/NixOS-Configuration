#!/usr/bin/env bash

CHOICE=$(zenity --list \
    --title="Ricing Selector" \
    --column="Theme" \
    --text="Choose the style:" \
    "Blue" \
    "Red" \
    "Purple" \
    " " \
    "Change Wallpaper" \
    " " \
    "Kill Rice" \
    --width=300 --height=450)

case "$CHOICE" in
    Blue)
        ~/dotfiles/scripts/blue.sh
        ;;
    Red)
        ~/dotfiles/scripts/red.sh
        ;;
    Purple)
        ~/dotfiles/scripts/purple.sh
        ;;
    "Change Wallpaper")
        ~/dotfiles/scripts/wallpaper.sh
        ;;
    "Kill Rice")
        pkill -f kitty
        ;;
esac