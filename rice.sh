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
    "Kill Rice")
        pkill -f kitty
        ;;
esac