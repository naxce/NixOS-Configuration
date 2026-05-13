# Zenith

Minimal Wayland compositor built for gaming performance. No wlroots. No bloat.
Direct DRM/KMS output, EGL/OpenGL ES rendering, libinput, Cairo UI.

---

## What Zenith Is

- A standalone Wayland compositor (window manager) that runs directly on DRM/KMS
- Designed exclusively for gaming — minimal overhead, no animations, no compositing effects
- Has a taskbar, desktop with icons, app launcher, monitor settings, and keybinds
- Works with NVIDIA drivers on Wayland
- Saves all settings to `~/.config/zenith/`
- Appears in SDDM as a session choice

## What Zenith Is Not

- Not a daily driver desktop (no notification daemon, no system tray, no widgets)
- Not a replacement for a full DE like KDE or GNOME

---

## Requirements

### Mandatory

| Dependency        | Package name (varies by distro)           |
| ----------------- | ----------------------------------------- |
| wayland           | `libwayland-dev` / `wayland-devel`        |
| wayland-scanner   | `wayland-scanner` / `wayland-devel`       |
| wayland-protocols | `wayland-protocols`                       |
| libdrm            | `libdrm-dev` / `libdrm-devel`             |
| mesa (EGL+GBM)    | `libgbm-dev` / `mesa-libGL-devel`         |
| libinput          | `libinput-dev` / `libinput-devel`         |
| libudev           | `libudev-dev` / `systemd-devel`           |
| cairo             | `libcairo2-dev` / `cairo-devel`           |
| pixman            | `libpixman-1-dev` / `pixman-devel`        |
| fontconfig        | `libfontconfig1-dev` / `fontconfig-devel` |
| cmake             | `cmake`                                   |
| pkg-config        | `pkg-config` / `pkgconf`                  |
| gcc               | `gcc`                                     |

### Optional but Recommended

| Package | Why                       |
| ------- | ------------------------- |
| `foot`  | Default terminal emulator |
| `sddm`  | Display manager for login |

---

## Building on Non-NixOS Linux (Ubuntu / Debian / Arch / Fedora / etc.)

### Step 1 — Install dependencies

**Ubuntu / Debian:**

```
sudo apt update
sudo apt install -y \
    libwayland-dev wayland-protocols wayland-scanner \
    libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev \
    libinput-dev libudev-dev \
    libcairo2-dev libpixman-1-dev libfontconfig1-dev \
    cmake pkg-config gcc make git
```

**Arch Linux:**

```
sudo pacman -S \
    wayland wayland-protocols \
    libdrm mesa libinput \
    cairo pixman fontconfig \
    cmake pkgconf gcc make git
```

**Fedora / RHEL:**

```
sudo dnf install -y \
    wayland-devel wayland-protocols-devel \
    libdrm-devel mesa-libGL-devel mesa-libEGL-devel \
    libgbm-devel libinput-devel systemd-devel \
    cairo-devel pixman-devel fontconfig-devel \
    cmake pkgconf gcc make git
```

### Step 2 — Clone and build

```
git clone https://github.com/naxce/zenith
cd zenith
bash scripts/build.sh
```

That is it. The binary ends up at `build/zenith`.

### Step 3 — Install system-wide

```
cd build
sudo make install
```

This installs:

- `/usr/local/bin/zenith`
- `/usr/local/share/wayland-sessions/zenith.desktop` (SDDM Wayland session)
- `/usr/local/share/xsessions/zenith.desktop` (SDDM X11 fallback session)

### Step 4 — Add to SDDM

If SDDM is already installed, the `.desktop` files installed above are enough.
SDDM will show "Zenith" and "Zenith (Wayland)" in the session list at next login.

If SDDM is not installed:

```
# Ubuntu / Debian
sudo apt install sddm

# Arch
sudo pacman -S sddm
sudo systemctl enable sddm

# Fedora
sudo dnf install sddm
sudo systemctl enable sddm
```

### Step 5 — Run Zenith

Log out and select "Zenith" from the SDDM session menu, or run directly from a TTY:

```
zenith
```

---

## Building on NixOS — Method 1: Flakes (recommended)

### Step 1 — Enable Flakes (if not already)

Add to your `/etc/nixos/configuration.nix`:

```nix
nix.settings.experimental-features = [ "nix-command" "flakes" ];
```

Then run `sudo nixos-rebuild switch`.

### Step 2 — Add Zenith as a flake input

In your system flake `flake.nix`:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    zenith.url = "path:/path/to/zenith";
    # or: zenith.url = "github:naxce/zenith";
  };

  outputs = { self, nixpkgs, zenith, ... }: {
    nixosConfigurations.myhost = nixpkgs.lib.nixosSystem {
      system = "x86_64-linux";
      modules = [
        zenith.nixosModules.default
        {
          programs.zenith = {
            enable = true;
            nvidia = true;
            terminal = "foot";
          };

          services.displayManager.sddm.enable = true;
          services.displayManager.sddm.wayland.enable = true;
        }
        ./hardware-configuration.nix
        ./configuration.nix
      ];
    };
  };
}
```

### Step 3 — Rebuild

```
sudo nixos-rebuild switch --flake .#myhost
```

---

## Building on NixOS — Method 2: No Flakes (classic)

### Step 1 — Build the package

```
cd /path/to/zenith
nix-build package.nix -I nixpkgs=channel:nixos-unstable
```

Result symlink: `./result/bin/zenith`

### Step 2 — Add to configuration

In `/etc/nixos/configuration.nix`:

```nix
{ config, pkgs, ... }:

let
  zenith = pkgs.callPackage /path/to/zenith/package.nix {};
in {
  environment.systemPackages = [ zenith ];

  services.displayManager.sessionPackages = [ zenith ];

  services.displayManager.sddm.enable = true;
  services.displayManager.sddm.wayland.enable = true;

  hardware.nvidia.modesetting.enable = true;

  boot.kernelParams = [
    "nvidia-drm.modeset=1"
    "nvidia-drm.fbdev=1"
  ];

  environment.sessionVariables = {
    LIBVA_DRIVER_NAME         = "nvidia";
    GBM_BACKEND               = "nvidia-drm";
    __GLX_VENDOR_LIBRARY_NAME = "nvidia";
    WLR_DRM_NO_ATOMIC         = "1";
    MOZ_ENABLE_WAYLAND        = "1";
    QT_QPA_PLATFORM           = "wayland";
    SDL_VIDEODRIVER           = "wayland";
  };
}
```

Then:

```
sudo nixos-rebuild switch
```

---

## Building on NixOS — Method 3: Home Manager

In your Home Manager config (`home.nix` or equivalent):

```nix
{ config, pkgs, ... }:

{
  imports = [
    (builtins.getFlake "path:/path/to/zenith").homeManagerModules.default
  ];

  wayland.windowManager.zenith = {
    enable = true;
    settings = {
      terminal = "foot";
      nvidia = true;
      gaps = 0;
    };
    autostart = [
      "foot --server"
    ];
  };
}
```

Run:

```
home-manager switch
```

---

## NVIDIA Setup (all distros)

These are required for NVIDIA + Wayland. Zenith sets them automatically, but also set them in your system/session environment:

```
nvidia-drm.modeset=1       # kernel parameter (boot)
nvidia-drm.fbdev=1         # kernel parameter (boot)

GBM_BACKEND=nvidia-drm
__GLX_VENDOR_LIBRARY_NAME=nvidia
LIBVA_DRIVER_NAME=nvidia
WLR_DRM_NO_ATOMIC=1
```

On NixOS the module handles this. On other distros add them to `/etc/environment` or your `.profile`:

```
echo 'GBM_BACKEND=nvidia-drm' | sudo tee -a /etc/environment
echo '__GLX_VENDOR_LIBRARY_NAME=nvidia' | sudo tee -a /etc/environment
echo 'LIBVA_DRIVER_NAME=nvidia' | sudo tee -a /etc/environment
echo 'WLR_DRM_NO_ATOMIC=1' | sudo tee -a /etc/environment
```

For the kernel parameters edit `/etc/default/grub`:

```
GRUB_CMDLINE_LINUX_DEFAULT="quiet nvidia-drm.modeset=1 nvidia-drm.fbdev=1"
```

Then run `sudo update-grub` and reboot.

---

## Configuration

All config lives in `~/.config/zenith/`.

### `~/.config/zenith/zenith.conf`

```
terminal=foot
wallpaper=/home/user/wallpaper.png
border_width=1
border_color_focused=FF4488FF
border_color_normal=FF222222
taskbar_bg=FF111111
taskbar_fg=FFEEEEEE
gaps=0
nvidia_mode=1
```

Colors are in `AARRGGBB` hex without `#`.

### `~/.config/zenith/autostart`

One command per line. Runs on startup. Lines starting with `#` are comments.

```
foot --server
```

### Desktop Icons

Put `.desktop` files or symlinks in `~/Desktop/`. They appear on the desktop.
Standard `.desktop` format:

```ini
[Desktop Entry]
Name=My Game
Exec=/path/to/game
Icon=/path/to/icon.png
Type=Application
```

---

## Keybinds

| Keys              | Action                  |
| ----------------- | ----------------------- |
| Super + Enter     | Open terminal           |
| Super + Space     | Open/close launcher     |
| Super + Shift + Q | Close focused window    |
| Super + H         | Minimize focused window |
| Super + M         | Maximize focused window |
| Super + D         | Minimize / restore all  |
| Alt + Tab         | Focus next window       |
| Alt + F4          | Close window            |
| F11               | Toggle fullscreen       |
| Super + Shift + R | Reload config           |
| Ctrl + Super + Q  | Quit Zenith             |

---

## Taskbar Buttons

| Button   | Action                                        |
| -------- | --------------------------------------------- |
| Settings | Opens monitor & config panel                  |
| Start    | Opens app launcher / search                   |
| Win-D    | Minimize all windows (click again to restore) |

**Clock** is shown on the right. Clicking window names in the taskbar focuses or minimizes them.

---

## Launcher (Start Menu)

- Open with `Super + Space` or the **Start** button
- Type to search installed apps (reads `.desktop` files)
- `Up`/`Down` to navigate, `Enter` to launch
- `Escape` to close
- **Shutdown / Reboot / Logout** buttons at the bottom

---

## Monitor Settings

Open with the **Settings** button or from the launcher.

- **Monitors tab**: lists all connected outputs, resolution, refresh rate, position, scale
  - Arrow keys: move monitor position
  - `+` / `-`: scale up/down
  - `E`: toggle enable/disable
  - `Enter` or "Save & Apply": save and apply
- **General tab**: shows current config values (edit the file directly for changes)
- **Keybinds tab**: reference list of all keybinds

Settings are saved to `~/.config/zenith/zenith.conf` and `monitor_N=` entries.

---

## Troubleshooting

### Zenith doesn't start / black screen

1. Check you have kernel modesetting: `cat /proc/cmdline | grep nvidia-drm`
2. Make sure no other compositor is running: `echo $WAYLAND_DISPLAY` should be empty
3. Check DRM access: `ls -la /dev/dri/` — your user needs to be in the `video` or `render` group
   ```
   sudo usermod -aG video,render $USER
   ```
   Log out and back in.
4. Run from TTY (not inside another compositor): switch to TTY2 with `Ctrl+Alt+F2`, log in, run `zenith`

### "No DRM device found"

You are probably trying to run Zenith inside an existing Wayland/X11 session.
Zenith must be started from a TTY or from a display manager (SDDM) on bare hardware.

### SDDM doesn't show Zenith

Make sure the `.desktop` file was installed:

```
ls /usr/local/share/wayland-sessions/
ls /usr/share/wayland-sessions/
```

If not, run `sudo make install` from the build directory.

### Build fails: "wayland-scanner not found"

Install `wayland-scanner`:

- Ubuntu: `sudo apt install wayland-scanner`
- Arch: included in `wayland` package
- Fedora: `sudo dnf install wayland-devel`

### Build fails: "xdg-shell.xml not found"

Run the protocol fetch script:

```
bash scripts/fetch-protocols.sh
```

Or manually copy from your system:

```
find /usr -name xdg-shell.xml 2>/dev/null
cp /path/found/xdg-shell.xml protocol/
```

### Apps don't use Wayland

Set these in your session or `~/.profile`:

```
export MOZ_ENABLE_WAYLAND=1
export QT_QPA_PLATFORM=wayland
export SDL_VIDEODRIVER=wayland
export CLUTTER_BACKEND=wayland
```

Zenith sets these automatically when it starts, but some apps read them before Zenith sets them.

### Cursor is invisible

This is a known NVIDIA Wayland issue. Add to your kernel parameters:

```
nvidia-drm.fbdev=1
```

### Games run slowly

Zenith has no compositor effects. If games still run slowly:

1. Use `gamemode` if available: prefix your game with `gamemoderun`
2. Set CPU governor: `sudo cpupower frequency-set -g performance`
3. Make sure the game uses Vulkan or native Wayland — not XWayland

---

## File Layout

```
zenith/
├── src/                  C source files
├── include/              Header files
├── protocol/             Wayland protocol XML + generated files
├── config/               Default config files
├── nixos/                NixOS module, Home Manager module, .desktop files
├── scripts/              Build and protocol helper scripts
├── CMakeLists.txt        Build system
├── flake.nix             Nix flake
├── package.nix           Nix package derivation
└── README.md             This file
```

---

## License

MIT
