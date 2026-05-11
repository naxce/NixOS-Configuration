#!/usr/bin/env bash

set -e

SSID="fatum"
PASS="Abc12345"
IFACE_AP="wlp16s0"
IFACE_INTERNET="enp6s0"

pkill create_ap 2>/dev/null || true
pkill wpa_supplicant 2>/dev/null || true
pkill NetworkManager 2>/dev/null || true

sudo ip link set "$IFACE_AP" down 2>/dev/null || true
sudo iw dev "$IFACE_AP" set type managed 2>/dev/null || true
sudo ip link set "$IFACE_AP" up

nohup sudo create_ap "$IFACE_AP" "$IFACE_INTERNET" "$SSID" "$PASS" \
  --freq-band 5 --no-virt > /tmp/hotspot.log 2>&1 &

disown

sleep 2

notify-send --app-name="Hotspot" "Hotspot Enabled 🚀" \
$'🛜 SSID: fatum\n🔑 Password: Abc12345'