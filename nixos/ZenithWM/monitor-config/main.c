/*
 * zenith-monitors - Monitor configuration GUI
 * Lists connected outputs, lets you set resolution/refresh/position/scale/rotation.
 * Saves to ~/.config/zenithwm/monitors.conf and signals compositor.
 *
 * Uses wlr-output-management-unstable-v1 protocol to talk to the compositor.
 * Falls back to reading /sys/class/drm if compositor is not running.
 */

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "monitor_config.h"

static void activate(GtkApplication *app, gpointer data) {
    (void)data;
    zenith_monitor_config_window(app);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "wm.zenith.monitors", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int r = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return r;
}