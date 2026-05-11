#!/bin/sh
printf 'timeout -1\ndefault_selection "Windows 11"\n' | sudo tee /boot/EFI/refind/manual_boot.conf && sudo systemctl reboot