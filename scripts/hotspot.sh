#!/usr/bin/env bash

set -e

SSID="fatum"
PASS="Abc12345"
IFACE_AP="wlp16s0"
IFACE_INTERNET="enp6s0"

if pgrep -x "create_ap" > /dev/null; then
    pkill create_ap
    systemctl restart NetworkManager
    notify-send --app-name="Hotspot" "Hotspot Disabled 🛑"
    exit 0
fi

pkill -9 wpa_supplicant || true
systemctl stop NetworkManager || true

ip link set "$IFACE_AP" down 2>/dev/null || true
iw dev "$IFACE_AP" set type managed 2>/dev/null || true
ip link set "$IFACE_AP" up

nohup create_ap "$IFACE_AP" "$IFACE_INTERNET" "$SSID" "$PASS" --freq-band 5 --no-virt > "/tmp/hotspot-$USER.log" 2>&1 &

disown
sleep 2

notify-send --app-name="Hotspot" "Hotspot Enabled 🚀" "🛜 SSID: $SSID\n🔑 Password: $PASS"