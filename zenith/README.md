<p align="center">
  <img src="https://via.placeholder.com/800x400?text=Zenith+Screenshot+Placeholder" alt="Zenith desktop preview">
</p>

# 🌄 Zenith – Minimal Gaming Desktop Environment

Zenith is a lightweight, fast, and gaming‑focused Wayland compositor built on **wlroots**.  
It features a clean panel, application launcher, desktop shortcuts, per‑output settings, and global keybindings – all with a tiny memory footprint and no unnecessary bloat.

> **Status:** Early development – stable enough for daily use, but expect occasional changes.

---

## ✨ Features

- 🎮 **Gaming‑oriented** – low latency, fullscreen toggling, minimise all
- 🖱️ **Desktop icons** – automatically scans `~/Desktop` for `.desktop` files
- 🔍 **Start menu** with search (work in progress)
- ⚙️ **Per‑output configuration** – position, scale, refresh rate
- ⌨️ **Global keybindings** (Super+Return → terminal, Super+Q → close window, etc.)
- 📦 **Single‑binary** – no heavyweight dependencies

---

## 📸 Screenshot / Demo

![Demo placeholder](https://via.placeholder.com/800x400?text=Animation+or+GIF+goes+here)

---

## 📋 Requirements

Build dependencies:

- `meson` & `ninja`
- `pkg-config`
- Wayland (`wayland`, `wayland-protocols`)
- `wlroots` (≥0.17)
- Cairo + Pango
- `libxkbcommon`
- `dbus`

Runtime dependencies:

- A graphics driver supporting EGL/GBM (i915, amdgpu, nouveau, etc.)
- `xkbcomp` (usually installed by default)

---

## 🔧 Cloning the repository

```bash
git clone https://github.com/naxce/zenith.git
cd zenith
```

---

## 🛠️ Compilation & Installation

### ❄️ NixOS (standalone):

```bash
nix-build -E 'with import <nixpkgs> {}; callPackage ./default.nix {}'
# result/bin/zenith
```

To install system‑wide, add to /etc/nixos/configuration.nix:

```nix
{ pkgs, ... }: {
  environment.systemPackages = [ (pkgs.callPackage /path/to/zenith {}) ];
}
```

---

### 🌨️ Nix Flakes + Home Manager:

Add to flake input:

```nix
inputs.zenith.url = "github:naxce/zenith";
```

Then enable the module in your Home Manager configuration:

```nix
{ inputs, ... }: {
  imports = [ inputs.zenith.homeManagerModules.zenith ];
  services.zenith.enable = true;
}
```

After rebuilding, the session will appear in your display manager.

---

## 🚀 Starting Zenith

### From a TTY

```bash
zenith
```

---

### Adding to SDDM (or any Wayland‑aware display manager)

During ninja install, a desktop entry is automatically placed in /usr/share/wayland-sessions/zenith.desktop.
If you used a package manager or manual install, ensure the file exists:

```bash
cat /usr/share/wayland-sessions/zenith.desktop
```

Should contain:

```ini
[Desktop Entry]
Name=Zenith
Comment=Minimal gaming desktop
Exec=zenith
Type=Application
DesktopNames=Zenith
```

Then restart SDDM and select Zenith from the session menu.

---

## ⚙️ Configuration

Zenith is configured via runtime files – no compile‑time changes needed.

_~/.config/zenith/_

_~/.config/zenith/monitors.conf_

---

Monitor config format (one line per output):

```text
<name> <x> <y> <scale> <refresh_mHz>
```

Example:

```text
DP-1 0 0 1.0 60000
HDMI-A-1 1920 0 1.5 59950
```
