<div align="center">

# The project is currently broken, I don't really have time to fix it yet, you can try yourself if you want or wait until I fix it.

```
███████╗███████╗███╗   ██╗██╗████████╗██╗  ██╗
╚══███╔╝██╔════╝████╗  ██║██║╚══██╔══╝██║  ██║
  ███╔╝ █████╗  ██╔██╗ ██║██║   ██║   ███████║
 ███╔╝  ██╔══╝  ██║╚██╗██║██║   ██║   ██╔══██║
███████╗███████╗██║ ╚████║██║   ██║   ██║  ██║
╚══════╝╚══════╝╚═╝  ╚═══╝╚═╝   ╚═╝   ╚═╝  ╚═╝
```

**Minimal Wayland compositor built for one thing — gaming.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)
[![NixOS](https://img.shields.io/badge/NixOS-supported-5277C3?style=flat-square&logo=nixos&logoColor=white)](flake.nix)
[![Wayland](https://img.shields.io/badge/Wayland-native-orange?style=flat-square)](https://wayland.freedesktop.org/)
[![NVIDIA](https://img.shields.io/badge/NVIDIA-ready-76b900?style=flat-square&logo=nvidia&logoColor=white)](README.md#nvidia-setup)
[![No wlroots](https://img.shields.io/badge/wlroots-free-red?style=flat-square)](README.md)

No wlroots. No bloat. No compositor effects. Direct DRM/KMS → EGL → screen.

</div>

---

## 📸 Screenshots

|                 Desktop                 |                 Launcher                  |             Monitor Settings              |
| :-------------------------------------: | :---------------------------------------: | :---------------------------------------: |
| ![Desktop](docs/screenshot-desktop.png) | ![Launcher](docs/screenshot-launcher.png) | ![Settings](docs/screenshot-settings.png) |
|       _Clean desktop with icons_        |         _App search + power menu_         |          _Monitor configuration_          |

---

## ✦ What is Zenith

Zenith is a **standalone Wayland compositor** that talks directly to the GPU via DRM/KMS and renders everything with OpenGL ES + Cairo. It has no dependency on wlroots.

It is **not** a daily driver. It is built exclusively to run games with the absolute minimum overhead.

| ✅ Has                          | ❌ Does not have         |
| ------------------------------- | ------------------------ |
| Taskbar with clock              | Notification daemon      |
| App launcher / start menu       | System tray              |
| Desktop with icons              | Widgets or applets       |
| Monitor configuration UI        | Compositor animations    |
| Window move / resize / minimize | Virtual desktops         |
| Keybinds                        | Built-in screenshot tool |
| NVIDIA Wayland support          | Bluetooth / network tray |
| SDDM session entry              | XWayland (add yourself)  |

---

## 🚀 Quick Start

### Option A — NixOS with Flakes _(recommended)_

```nix
# flake.nix
inputs.zenith.url = "github:naxce/zenith";
# or local: inputs.zenith.url = "path:/path/to/zenith";
```

```nix
# configuration.nix
imports = [ zenith.nixosModules.default ];

programs.zenith = {
  enable   = true;
  nvidia   = true;
  terminal = "kitty";
};

services.displayManager.sessionPackages = [ zenith.packages.x86_64-linux.default ];
services.displayManager.sddm.enable         = true;
services.displayManager.sddm.wayland.enable = true;
```

```bash
sudo nixos-rebuild switch --flake .#yourhost
```

### Option B — Non-NixOS (Arch / Debian / Fedora)

```bash
git clone https://github.com/naxce/zenith && cd zenith
bash scripts/build.sh
cd build && sudo make install
```

> **Full instructions for every method** are in the [Installation](#-installation) section below.

---

## ⌨️ Keybinds

| Keys                | Action                  |
| ------------------- | ----------------------- |
| `Super + Enter`     | Open terminal           |
| `Super + Space`     | Toggle launcher         |
| `Super + Shift + Q` | Close focused window    |
| `Super + H`         | Minimize focused window |
| `Super + M`         | Maximize focused window |
| `Super + D`         | Minimize / restore all  |
| `Alt + Tab`         | Focus next window       |
| `Alt + F4`          | Close window            |
| `F11`               | Toggle fullscreen       |
| `Super + Shift + R` | Reload config           |
| `Ctrl + Super + Q`  | Quit Zenith             |

---

## 🖥️ Taskbar

```
[ Settings ] [ Start ] [ Win-D ]   [ window1 ] [ window2 ] ...   2025-05-14  23:41:07
```

| Button       | Action                                        |
| ------------ | --------------------------------------------- |
| **Settings** | Open monitor & general settings panel         |
| **Start**    | Open app launcher / start menu                |
| **Win-D**    | Minimize all windows (click again to restore) |

Window buttons in the middle: **click** to focus, **click active** to minimize.

---

## 📦 Installation

### Requirements

<details>
<summary><b>📋 Full dependency list</b></summary>

| Dependency        | Ubuntu / Debian               | Arch                | Fedora                           |
| ----------------- | ----------------------------- | ------------------- | -------------------------------- |
| wayland           | `libwayland-dev`              | `wayland`           | `wayland-devel`                  |
| wayland-scanner   | `wayland-scanner`             | `wayland`           | `wayland-devel`                  |
| wayland-protocols | `wayland-protocols`           | `wayland-protocols` | `wayland-protocols-devel`        |
| libdrm            | `libdrm-dev`                  | `libdrm`            | `libdrm-devel`                   |
| EGL / GBM         | `libegl1-mesa-dev libgbm-dev` | `mesa`              | `mesa-libEGL-devel libgbm-devel` |
| GLES2             | `libgles2-mesa-dev`           | `mesa`              | `mesa-libGLES-devel`             |
| libinput          | `libinput-dev`                | `libinput`          | `libinput-devel`                 |
| libudev           | `libudev-dev`                 | `systemd`           | `systemd-devel`                  |
| cairo             | `libcairo2-dev`               | `cairo`             | `cairo-devel`                    |
| pixman            | `libpixman-1-dev`             | `pixman`            | `pixman-devel`                   |
| fontconfig        | `libfontconfig1-dev`          | `fontconfig`        | `fontconfig-devel`               |
| cmake             | `cmake`                       | `cmake`             | `cmake`                          |
| pkg-config        | `pkg-config`                  | `pkgconf`           | `pkgconf`                        |
| gcc               | `gcc`                         | `gcc`               | `gcc`                            |

</details>

---

### 🐧 Method 1 — Non-NixOS Linux

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y \
  libwayland-dev wayland-protocols \
  libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev \
  libinput-dev libudev-dev \
  libcairo2-dev libpixman-1-dev libfontconfig1-dev \
  cmake pkg-config gcc make git
```

#### Arch Linux

```bash
sudo pacman -S \
  wayland wayland-protocols \
  libdrm mesa libinput \
  cairo pixman fontconfig \
  cmake pkgconf gcc make git
```

#### Fedora / RHEL

```bash
sudo dnf install -y \
  wayland-devel wayland-protocols-devel \
  libdrm-devel mesa-libEGL-devel libgbm-devel mesa-libGLES-devel \
  libinput-devel systemd-devel \
  cairo-devel pixman-devel fontconfig-devel \
  cmake pkgconf gcc make git
```

#### Build & Install

```bash
git clone https://github.com/naxce/zenith
cd zenith
bash scripts/build.sh        # fetches protocols, runs cmake + make
cd build
sudo make install            # installs to /usr/local/bin and drops .desktop files
```

#### Add to SDDM

```bash
# Install SDDM if needed
sudo apt install sddm        # Debian/Ubuntu
sudo pacman -S sddm          # Arch
sudo dnf install sddm        # Fedora

sudo systemctl enable --now sddm
```

The `.desktop` files installed by `make install` are picked up by SDDM automatically. Reboot and select **Zenith** from the session list.

---

### ❄️ Method 2 — NixOS with Flakes _(recommended)_

> **This is the tested, working method.** Follow every step exactly.

#### Step 1 — Enable Flakes

In `/etc/nixos/configuration.nix` (or your flake config):

```nix
nix.settings.experimental-features = [ "nix-command" "flakes" ];
```

```bash
sudo nixos-rebuild switch
```

#### Step 2 — Add to your flake inputs

```nix
# flake.nix
{
  inputs = {
    nixpkgs.url      = "github:nixos/nixpkgs/nixos-unstable";
    home-manager     = { url = "github:nix-community/home-manager"; inputs.nixpkgs.follows = "nixpkgs"; };
    zenith.url       = "github:naxce/zenith";
    # local development:
    # zenith.url     = "path:/home/youruser/dotfiles/zenith";
  };

  outputs = { self, nixpkgs, home-manager, zenith, ... }: {
    nixosConfigurations.YOURHOSTNAME = nixpkgs.lib.nixosSystem {
      system = "x86_64-linux";
      specialArgs = { inherit zenith; };
      modules = [
        zenith.nixosModules.default
        ./nixos/configuration.nix
        home-manager.nixosModules.home-manager
        { home-manager.users.YOURUSER.imports = [ ./home.nix ]; }
      ];
    };
  };
}
```

> Replace `YOURHOSTNAME` and `YOURUSER` with your actual values.

#### Step 3 — Enable in configuration.nix

```nix
# nixos/configuration.nix
{ config, pkgs, zenith, ... }:

{
  programs.zenith = {
    enable   = true;
    nvidia   = true;        # set false if you don't have NVIDIA
    terminal = "kitty";
  };

  services.displayManager.sessionPackages = [
    zenith.packages.x86_64-linux.default
  ];

  services.displayManager.sddm.enable         = true;
  services.displayManager.sddm.wayland.enable = true;
}
```

> ⚠️ **The `sessionPackages` line is mandatory.** Without it, SDDM will not show Zenith in the session list even if the package is installed.

#### Step 4 — Rebuild

```bash
sudo nixos-rebuild switch --flake .#YOURHOSTNAME
```

#### Step 5 — Verify

```bash
# Should show zenith.desktop
ls $(grep "SessionDir.*wayland" /etc/sddm.conf.d/*.conf | cut -d= -f2)/

# Should print the zenith binary path
which zenith
```

Reboot and select **Zenith** from the SDDM session list.

---

### ❄️ Method 3 — NixOS without Flakes

```bash
cd /path/to/zenith
nix-build package.nix -I nixpkgs=channel:nixos-unstable
# binary at ./result/bin/zenith
```

In `/etc/nixos/configuration.nix`:

```nix
{ config, pkgs, ... }:

let
  zenith = pkgs.callPackage /path/to/zenith/package.nix {};
in {
  environment.systemPackages = [ zenith ];

  services.displayManager.sessionPackages     = [ zenith ];
  services.displayManager.sddm.enable         = true;
  services.displayManager.sddm.wayland.enable = true;
}
```

```bash
sudo nixos-rebuild switch
```

---

### ❄️ Method 4 — NixOS with Home Manager

```nix
# home.nix
{ zenith, ... }:

{
  imports = [ zenith.homeManagerModules.default ];

  wayland.windowManager.zenith = {
    enable = true;
    settings = {
      terminal    = "kitty";
      nvidia      = true;
      gaps        = 0;
    };
    autostart = [
      "kitty --server"
    ];
  };
}
```

> Note: Home Manager manages `~/.config/zenith/` but the NixOS module still handles SDDM registration. You need both.

---

## 🟢 NVIDIA Setup

Zenith sets all required env vars automatically on startup. For system-level setup:

#### NixOS

The `programs.zenith.nvidia = true` option handles everything. Additionally ensure:

```nix
hardware.nvidia.modesetting.enable = true;
boot.kernelParams = [ "nvidia-drm.modeset=1" "nvidia-drm.fbdev=1" ];
```

#### Other distros

Add to `/etc/environment`:

```
GBM_BACKEND=nvidia-drm
__GLX_VENDOR_LIBRARY_NAME=nvidia
LIBVA_DRIVER_NAME=nvidia
WLR_DRM_NO_ATOMIC=1
```

Add to kernel parameters (edit `/etc/default/grub`):

```
GRUB_CMDLINE_LINUX_DEFAULT="quiet nvidia-drm.modeset=1 nvidia-drm.fbdev=1"
```

```bash
sudo update-grub && reboot
```

---

## ⚙️ Configuration

All files live in `~/.config/zenith/`.

### `zenith.conf`

```ini
terminal=kitty
wallpaper=/home/user/pictures/wallpaper.png
border_width=1
border_color_focused=FF4488FF
border_color_normal=FF222222
taskbar_bg=FF111111
taskbar_fg=FFEEEEEE
gaps=0
nvidia_mode=1
```

> Colors are `AARRGGBB` hex without `#`. Reload with `Super + Shift + R`.

### `autostart`

One command per line, executed on Zenith startup:

```bash
kitty --server
```

### Desktop Icons

Drop `.desktop` files or symlinks in `~/Desktop/`:

```ini
[Desktop Entry]
Name=My Game
Exec=/path/to/game
Icon=/path/to/icon.png
Type=Application
```

---

## 🖥️ Monitor Settings Panel

Open with the **Settings** button on the taskbar.

| Tab          | What you can do                                                                                                       |
| ------------ | --------------------------------------------------------------------------------------------------------------------- |
| **Monitors** | See all connected outputs. Arrow keys = move position. `+`/`-` = scale. `E` = enable/disable. `Enter` = save & apply. |
| **General**  | View current config values (edit the file directly to change).                                                        |
| **Keybinds** | Reference list of all keybinds.                                                                                       |

Settings are written back to `~/.config/zenith/zenith.conf`.

---

## 🔧 Troubleshooting

<details>
<summary><b>❌ Zenith not showing in SDDM</b></summary>

**On NixOS:** Make sure you have this line in `configuration.nix`:

```nix
services.displayManager.sessionPackages = [
  zenith.packages.x86_64-linux.default
];
```

This is the most common mistake. The module alone is not enough — SDDM needs the package explicitly registered.

Verify after rebuild:

```bash
ls $(grep "SessionDir.*wayland" /etc/sddm.conf.d/*.conf | cut -d= -f2)/
```

`zenith.desktop` must appear in that output.

**On other distros:** Verify the `.desktop` file exists:

```bash
ls /usr/local/share/wayland-sessions/zenith.desktop
ls /usr/share/wayland-sessions/zenith.desktop
```

If missing, run `sudo make install` from the build directory.

</details>

<details>
<summary><b>❌ Black screen / Zenith doesn't start</b></summary>

1. Check kernel modesetting is active:
   ```bash
   cat /proc/cmdline | grep nvidia-drm
   ```
2. Make sure you're not inside another compositor — run from a TTY (`Ctrl+Alt+F2`)
3. Check DRM group membership:
   ```bash
   groups $USER   # should include 'video' and/or 'render'
   sudo usermod -aG video,render $USER  # then log out and back in
   ```
4. Run directly and check output:
   ```bash
   zenith 2>&1 | head -20
   ```

</details>

<details>
<summary><b>❌ "No DRM device found"</b></summary>

You are running Zenith inside an existing Wayland or X11 session. Zenith must run on bare hardware — from a TTY or from SDDM, not nested.

</details>

<details>
<summary><b>❌ Build fails: egl / gbm not found (non-NixOS)</b></summary>

Install the Mesa dev packages for your distro:

```bash
# Ubuntu / Debian
sudo apt install libegl1-mesa-dev libgbm-dev

# Arch
sudo pacman -S mesa

# Fedora
sudo dnf install mesa-libEGL-devel libgbm-devel
```

</details>

<details>
<summary><b>❌ Build fails: xdg-shell.xml not found</b></summary>

Run the fetch script:

```bash
bash scripts/fetch-protocols.sh
```

Or copy manually:

```bash
find /usr -name xdg-shell.xml 2>/dev/null
# then:
cp /path/to/xdg-shell.xml protocol/
```

</details>

<details>
<summary><b>❌ Invisible cursor (NVIDIA)</b></summary>

Add to kernel parameters:

```
nvidia-drm.fbdev=1
```

On NixOS: `boot.kernelParams = [ "nvidia-drm.fbdev=1" ];`

</details>

<details>
<summary><b>❌ Apps not using Wayland</b></summary>

Zenith sets these automatically, but some apps read them before launch. Add to `~/.profile` or `/etc/environment`:

```bash
export MOZ_ENABLE_WAYLAND=1
export QT_QPA_PLATFORM=wayland
export SDL_VIDEODRIVER=wayland
export CLUTTER_BACKEND=wayland
```

</details>

---

## 📁 Project Structure

```
zenith/
├── src/                    C source files
│   ├── zenith.c            Entry point, main loop
│   ├── output.c            DRM/KMS + EGL output management
│   ├── input.c             libinput keyboard/mouse handling
│   ├── window.c            Window management
│   ├── taskbar.c           Taskbar rendering (Cairo)
│   ├── desktop.c           Desktop + icon rendering
│   ├── launcher.c          App launcher / start menu
│   ├── settings_panel.c    Monitor & config settings UI
│   ├── keybinds.c          Keyboard shortcut handling
│   ├── config.c            Config file read/write
│   └── monitor_settings.c  Monitor apply/save/load
├── include/                Header files
├── protocol/               Wayland protocol XML + generated files
├── config/                 Default config files
├── nixos/
│   ├── module.nix          NixOS module  (programs.zenith.*)
│   ├── home-manager.nix    Home Manager module
│   ├── zenith.desktop      Wayland session entry for SDDM
│   └── zenith-sddm.desktop X11 session entry for SDDM
├── scripts/
│   ├── build.sh            One-command build for non-NixOS
│   └── fetch-protocols.sh  Downloads xdg-shell.xml
├── CMakeLists.txt
├── flake.nix
├── package.nix
└── README.md
```

---

## 🎮 Gaming Tips

- Use `gamemoderun %command%` in Steam launch options for CPU governor boost
- Set `nvidia_mode=1` in `zenith.conf` (default)
- Prefer Vulkan / native Wayland games over OpenGL / XWayland for best performance
- `gaps=0` and `border_width=1` give the smallest possible compositor overhead
- Zenith renders the taskbar at 60fps and nothing else unless windows are present

---

<div align="center">

**MIT License** · Built for NixOS · No wlroots · NVIDIA ready

_If Zenith helped you get more frames, leave a ⭐_

</div>
# Zenith
# Zenith
