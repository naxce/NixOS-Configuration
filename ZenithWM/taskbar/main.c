/*
 * ZenithWM Taskbar - main.c
 * GTK4 layer-shell taskbar: clock, settings, start menu, show-desktop.
 */

#include <gtk/gtk.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <glib.h>
#include "taskbar.h"

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    zenith_taskbar_create(app);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "wm.zenith.taskbar", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}