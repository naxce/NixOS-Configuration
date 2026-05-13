#!/usr/bin/env bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROTO_DIR="$SCRIPT_DIR/protocol"
mkdir -p "$PROTO_DIR"

XDG_SHELL_URL="https://gitlab.freedesktop.org/wayland/wayland-protocols/-/raw/main/stable/xdg-shell/xdg-shell.xml"

if command -v pkg-config &>/dev/null; then
    PROTO_PATH=$(pkg-config --variable=pkgdatadir wayland-protocols 2>/dev/null || true)
    if [ -n "$PROTO_PATH" ] && [ -f "$PROTO_PATH/stable/xdg-shell/xdg-shell.xml" ]; then
        cp "$PROTO_PATH/stable/xdg-shell/xdg-shell.xml" "$PROTO_DIR/xdg-shell.xml"
        echo "Copied xdg-shell.xml from $PROTO_PATH"
        exit 0
    fi
fi

for d in /usr/share/wayland-protocols /usr/local/share/wayland-protocols \
          /run/current-system/sw/share/wayland-protocols; do
    if [ -f "$d/stable/xdg-shell/xdg-shell.xml" ]; then
        cp "$d/stable/xdg-shell/xdg-shell.xml" "$PROTO_DIR/xdg-shell.xml"
        echo "Copied xdg-shell.xml from $d"
        exit 0
    fi
done

echo "Downloading xdg-shell.xml..."
if command -v curl &>/dev/null; then
    curl -fsSL "$XDG_SHELL_URL" -o "$PROTO_DIR/xdg-shell.xml"
elif command -v wget &>/dev/null; then
    wget -q "$XDG_SHELL_URL" -O "$PROTO_DIR/xdg-shell.xml"
else
    echo "ERROR: Cannot find xdg-shell.xml and no curl/wget available."
    echo "Install wayland-protocols package or download manually from:"
    echo "$XDG_SHELL_URL"
    exit 1
fi
echo "Downloaded xdg-shell.xml"
