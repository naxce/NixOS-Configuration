#!/usr/bin/env bash

CHOICE=$(zenity --list \
    --title="Wallpaper Selector" \
    --column="Wallpaper" \
    --text="Choose wallpaper:" \
    "Blue" \
    "Red" \
    "Purple" \
    " " \
    "Choose Ricing Theme" \
    " " \
    "Kill Rice" \
    --width=300 --height=450)

case "$CHOICE" in
    Blue)
        pkill -f kitty
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Blue.jpg
        ;;
    Red)
        pkill -f kitty
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Red.jpg
        ;;
    Purple)
        pkill -f kitty
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Purple.jpg
        ;;
    "Choose Ricing Theme")
        ~/dotfiles/scripts/rice.sh
        ;;
    "Kill Rice")
        pkill -f kitty
        ;;
esac