#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <limits.h>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <xdg-shell-server-protocol.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gbm.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libinput.h>
#include <libudev.h>
#include <libseat.h>

#include <pixman-1/pixman.h>
#include <fontconfig/fontconfig.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "zenith.h"
#include "config.h"
#include "taskbar.h"
#include "desktop.h"
#include "input.h"
#include "output.h"
#include "window.h"
#include "monitor_settings.h"
#include "keybinds.h"
#include "launcher.h"
#include "settings_panel.h"

static struct zenith_server server = {0};
static volatile int running = 1;

static void handle_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void handle_sigterm(int sig) {
    (void)sig;
    running = 0;
    wl_display_terminate(server.display);
}

static void setup_signals(void) {
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);
    sa.sa_handler = handle_sigterm;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);
}

static void spawn_autostart(void) {
    char path[PATH_MAX];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.config/zenith/autostart", home);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;
        pid_t pid = fork();
        if (pid == 0) {
            setsid();
            execlp("/bin/sh", "sh", "-c", line, NULL);
            _exit(1);
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    setup_signals();
    zenith_config_init(&server.config);
    zenith_config_load(&server.config);

    server.display = wl_display_create();
    if (!server.display) {
        fprintf(stderr, "zenith: failed to create wayland display\n");
        return 1;
    }

    server.event_loop = wl_display_get_event_loop(server.display);

    if (!zenith_output_init(&server)) {
        fprintf(stderr, "zenith: failed to init output\n");
        return 1;
    }

    if (!zenith_input_init(&server)) {
        fprintf(stderr, "zenith: failed to init input\n");
        return 1;
    }

    if (!zenith_windows_init(&server)) {
        fprintf(stderr, "zenith: failed to init windows\n");
        return 1;
    }

    if (!zenith_desktop_init(&server)) {
        fprintf(stderr, "zenith: failed to init desktop\n");
        return 1;
    }

    if (!zenith_taskbar_init(&server)) {
        fprintf(stderr, "zenith: failed to init taskbar\n");
        return 1;
    }

    if (!zenith_keybinds_init(&server)) {
        fprintf(stderr, "zenith: failed to init keybinds\n");
        return 1;
    }

    if (!zenith_launcher_init(&server)) {
        fprintf(stderr, "zenith: failed to init launcher\n");
        return 1;
    }

    if (!zenith_settings_panel_init(&server)) {
        fprintf(stderr, "zenith: failed to init settings panel\n");
        return 1;
    }

    const char *socket = wl_display_add_socket_auto(server.display);
    if (!socket) {
        fprintf(stderr, "zenith: failed to create wayland socket\n");
        return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, 1);
    setenv("XDG_SESSION_TYPE", "wayland", 1);
    setenv("__GL_YIELD", "USLEEP", 1);
    setenv("__GL_MaxFramesAllowed", "1", 1);
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("WLR_DRM_NO_ATOMIC", "1", 1);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1);
    setenv("SDL_VIDEODRIVER", "wayland", 1);
    setenv("CLUTTER_BACKEND", "wayland", 1);

    spawn_autostart();
    zenith_taskbar_spawn(&server);

    fprintf(stdout, "zenith: running on %s\n", socket);

    while (running) {
        wl_event_loop_dispatch(server.event_loop, 16);
        wl_display_flush_clients(server.display);
    }

    zenith_settings_panel_destroy(&server);
    zenith_launcher_destroy(&server);
    zenith_keybinds_destroy(&server);
    zenith_taskbar_destroy(&server);
    zenith_desktop_destroy(&server);
    zenith_windows_destroy(&server);
    zenith_input_destroy(&server);
    zenith_output_destroy(&server);
    zenith_config_save(&server.config);
    wl_display_destroy(server.display);

    return 0;
}
