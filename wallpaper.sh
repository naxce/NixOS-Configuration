#!/usr/bin/env bash

pkill -f kitty

CHOICE=$(zenity --list \
    --title="Wallpaper Selector" \
    --column="Wallpaper" \
    --text="Choose wallpaper:" \
    "Blue" \
    "Red" \
    "Purple" \
    " " \
    "Choose Ricing Theme" \
    --width=300 --height=450)

case "$CHOICE" in
    Blue)
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Blue.jpg
        ;;
    Red)
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Red.jpg
        ;;
    Purple)
        plasma-apply-wallpaperimage ~/dotfiles/Pictures/Purple.jpg
        ;;
    "Choose Ricing Theme")
        ~/dotfiles/rice.sh
        ;;
esac