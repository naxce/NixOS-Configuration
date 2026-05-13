# ZenithWM

A minimal Wayland compositor built for gaming performance. No bloat, no compositing overhead, no distractions.

## Design Philosophy

- **Zero background processes** beyond what's needed
- **No vsync tearing** by default (can be disabled for competitive gaming)
- **NVIDIA-first**: works with proprietary drivers out of the box
- **Wayland native**: no XWayland overhead for native Wayland games
- XWayland available for legacy games

## Features

- Minimal compositor (wlroots-based)
- Taskbar with: clock, settings, start menu (app launcher + shutdown/reboot/logout), show desktop
- Desktop icons (configurable via `~/.config/zenithwm/desktop.conf`)
- Keybindings (configurable via `~/.config/zenithwm/keys.conf`)
- Monitor configuration GUI (wdisplay-style)
- Persistent settings
- SDDM session entry
- NVIDIA + Wayland support via env vars

## File Structure

```
ZenithWM/
├── compositor/          # wlroots-based Wayland compositor (C)
│   ├── main.c
│   ├── server.c / server.h
│   ├── output.c / output.h
│   ├── input.c / input.h
│   ├── xdg_shell.c / xdg_shell.h
│   └── meson.build
├── taskbar/             # GTK4 taskbar (C)
│   ├── main.c
│   ├── taskbar.c / taskbar.h
│   ├── clock.c / clock.h
│   ├── startmenu.c / startmenu.h
│   ├── settings_win.c / settings_win.h
│   └── meson.build
├── monitor-config/      # Monitor configuration tool (C + GTK4)
│   ├── main.c
│   ├── monitor_config.c / monitor_config.h
│   └── meson.build
├── config/              # Default config files
│   ├── zenithwm.conf
│   ├── keys.conf
│   └── desktop.conf
├── session/             # Session scripts
│   ├── zenithwm-session
│   └── zenithwm.desktop
├── sddm/
│   └── zenithwm.desktop
├── nix/
│   ├── default.nix      # Nix package
│   └── module.nix       # NixOS module
├── meson.build          # Root build file
└── README.md
```

## Building on NixOS

### Option 1: NixOS Module (recommended)

Add to your `configuration.nix`:

```nix
{ config, pkgs, ... }:
let
  zenithwm = import /path/to/ZenithWM/nix/default.nix { inherit pkgs; };
in {
  imports = [ /path/to/ZenithWM/nix/module.nix ];

  services.zenithwm.enable = true;
  # Optionally set nvidia mode:
  # services.zenithwm.nvidia = true;
}
```

Then `sudo nixos-rebuild switch`.

### Option 2: nix-build

```bash
cd ZenithWM
nix-build nix/default.nix
# Result symlinked to ./result
```

### Option 3: nix develop + meson (manual)

```bash
nix develop  # or nix-shell if using nix/default.nix as shell.nix
meson setup build
ninja -C build
sudo ninja -C build install
```

## Building without Nix (Arch/Debian)

Dependencies:

- wlroots >= 0.17
- wayland-server
- wayland-protocols
- libxkbcommon
- gtk4
- meson, ninja

```bash
meson setup build --prefix=/usr
ninja -C build
sudo ninja -C build install
```

## Configuration

### Main config: `~/.config/zenithwm/zenithwm.conf`

```ini
[compositor]
# VRR/FreeSync (adaptive sync)
adaptive_sync = true
# Disable vsync for tear-free or enable for competitive (less latency)
vsync = false
# XWayland for legacy games
xwayland = true
# Gap between windows (set 0 for pure performance)
border_width = 1
border_color = #333333

[performance]
# Disable compositor effects entirely
no_animations = true
# CPU governor hint (informational, set via your system)
governor_hint = performance
```

### Keybindings: `~/.config/zenithwm/keys.conf`

```ini
# Format: MOD+KEY = action
# MOD = Super (Win key)

Super+Return   = exec foot
Super+q        = close_window
Super+f        = fullscreen
Super+m        = minimize
Super+d        = show_desktop
Super+r        = exec zenithwm-run
Super+l        = exec zenithwm-lock
Super+1        = workspace 1
Super+2        = workspace 2
Super+3        = workspace 3
Super+Shift+1  = move_to_workspace 1
Super+Shift+2  = move_to_workspace 2
Super+Shift+3  = move_to_workspace 3
```

### Desktop icons: `~/.config/zenithwm/desktop.conf`

```ini
[icon]
name = Steam
exec = steam
icon = /usr/share/icons/hicolor/256x256/apps/steam.png
x = 50
y = 50

[icon]
name = Terminal
exec = foot
icon = /usr/share/icons/hicolor/scalable/apps/foot.svg
x = 50
y = 150
```

## NVIDIA Setup

ZenithWM sets required NVIDIA+Wayland env vars automatically when launched via SDDM or the session script. If running manually:

```bash
export LIBVA_DRIVER_NAME=nvidia
export GBM_BACKEND=nvidia-drm
export __GLX_VENDOR_LIBRARY_NAME=nvidia
export WLR_NO_HARDWARE_CURSORS=1
export WLR_RENDERER=vulkan
zenithwm-session
```

## Monitor Configuration

Launch the monitor config tool:

- From taskbar: Settings → Monitor Configuration
- Or run: `zenith-monitors`

Supports: resolution, refresh rate, position, rotation, scale, primary monitor. Settings saved to `~/.config/zenithwm/monitors.conf`.
