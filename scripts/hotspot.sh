#!/usr/bin/env bash

set -e

SSID="fatum"
PASS="Abc12345"
IFACE_AP="wlp16s0"
IFACE_INTERNET="enp6s0"

if pgrep -x "create_ap" > /dev/null; then
    echo "Stopping Hotspot... 🛑"
    sudo pkill create_ap
    sudo systemctl restart NetworkManager
    notify-send --app-name="Hotspot" "Hotspot Disabled 🛑" "Wi-Fi should be back to normal!"
    exit 0
fi

echo "Starting Hotspot... 🚀"

sudo pkill -9 wpa_supplicant || true
sudo systemctl stop NetworkManager || true

sudo ip link set "$IFACE_AP" down 2>/dev/null || true
sudo iw dev "$IFACE_AP" set type managed 2>/dev/null || true
sudo ip link set "$IFACE_AP" up

nohup sudo create_ap "$IFACE_AP" "$IFACE_INTERNET" "$SSID" "$PASS" --freq-band 5 --no-virt > /tmp/hotspot.log 2>&1 &

disown

sleep 2

notify-send --app-name="Hotspot" "Hotspot Enabled 🚀" "🛜 SSID: $SSID\n🔑 Password: $PASS"