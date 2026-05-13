/*
 * ZenithWM - Minimal Wayland compositor for gaming
 * main.c - Entry point
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

#include "server.h"

static void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

static void sigusr1_handler(int sig) {
    (void)sig;
    /* Triggered by taskbar/external tools to reload config */
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    /* Reap children (spawned apps) */
    signal(SIGCHLD, sigchld_handler);
    signal(SIGUSR1, sigusr1_handler);

    /* Set NVIDIA+Wayland env vars if not already set */
    if (!getenv("GBM_BACKEND"))
        setenv("GBM_BACKEND", "nvidia-drm", 0);
    if (!getenv("__GLX_VENDOR_LIBRARY_NAME"))
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 0);
    if (!getenv("WLR_NO_HARDWARE_CURSORS"))
        setenv("WLR_NO_HARDWARE_CURSORS", "1", 0);
    if (!getenv("WLR_RENDERER"))
        setenv("WLR_RENDERER", "vulkan", 0);
    if (!getenv("LIBVA_DRIVER_NAME"))
        setenv("LIBVA_DRIVER_NAME", "nvidia", 0);

    /* Apps launched from zenithwm inherit Wayland socket */
    setenv("XDG_SESSION_TYPE", "wayland", 1);
    setenv("MOZ_ENABLE_WAYLAND", "1", 0);
    setenv("QT_QPA_PLATFORM", "wayland", 0);

    wlr_log_init(WLR_INFO, NULL);

    struct zenith_server server = {0};

    if (!zenith_server_init(&server)) {
        wlr_log(WLR_ERROR, "Failed to initialize ZenithWM server");
        return EXIT_FAILURE;
    }

    /* Set WAYLAND_DISPLAY so child processes can connect */
    const char *socket = wl_display_get_name(server.wl_display);
    setenv("WAYLAND_DISPLAY", socket, 1);
    wlr_log(WLR_INFO, "ZenithWM running on %s", socket);

    /* Launch taskbar */
    pid_t taskbar_pid = fork();
    if (taskbar_pid == 0) {
        execlp("zenith-taskbar", "zenith-taskbar", NULL);
        _exit(1);
    }

    zenith_server_run(&server);
    zenith_server_destroy(&server);

    return EXIT_SUCCESS;
}