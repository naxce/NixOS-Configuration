#ifndef ZENITH_H
#define ZENITH_H

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libinput.h>
#include <libudev.h>
#include <libseat.h>
#include <cairo/cairo.h>
#include <pixman-1/pixman.h>

#include "config.h"

#define ZENITH_MAX_OUTPUTS 8
#define ZENITH_MAX_WINDOWS 256
#define ZENITH_MAX_KEYBINDS 64
#define ZENITH_TASKBAR_HEIGHT 36
#define ZENITH_VERSION "1.0.0"

struct zenith_config;
struct zenith_output;
struct zenith_window;
struct zenith_taskbar;
struct zenith_desktop;
struct zenith_input;
struct zenith_keybinds;
struct zenith_launcher;
struct zenith_settings_panel;

struct zenith_server {
    struct wl_display *display;
    struct wl_event_loop *event_loop;

    struct zenith_config config;

    struct zenith_output *outputs[ZENITH_MAX_OUTPUTS];
    int output_count;

    struct zenith_window *windows[ZENITH_MAX_WINDOWS];
    int window_count;
    struct zenith_window *focused;

    struct zenith_taskbar *taskbar;
    struct zenith_desktop *desktop;
    struct zenith_input *input;
    struct zenith_keybinds *keybinds;
    struct zenith_launcher *launcher;
    struct zenith_settings_panel *settings_panel;

    struct wl_global *compositor_global;
    struct wl_global *xdg_shell_global;
    struct wl_global *seat_global;
    struct wl_global *output_global;

    int drm_fd;
    struct gbm_device *gbm;
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLConfig egl_config;

    struct udev *udev;
    struct libinput *libinput;
    struct libseat *seat;

    int launcher_visible;
    int settings_visible;
    int all_minimized;

    uint32_t last_frame_time;
};

struct zenith_rect {
    int x, y, w, h;
};

struct zenith_color {
    float r, g, b, a;
};

#endif
