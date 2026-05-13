/*
 * ZenithWM Taskbar - taskbar.c
 * Creates the layer-shell window, lays out buttons.
 *
 * Layout (left to right):
 *   [⊞ Start]  [window buttons...]  <spacer>  [🖥 Monitors] [⚙ Settings]  [HH:MM]
 *
 * The taskbar registers itself via wlr-layer-shell so it stays on top
 * and the compositor knows to reserve 32px at the top of each output.
 */

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <stdlib.h>
#include <string.h>
#include "taskbar.h"
#include "clock.h"
#include "startmenu.h"
#include "settings_win.h"

/* CSS for the entire taskbar */
static const char *TASKBAR_CSS =
    "window.taskbar {"
    "  background-color: #111111;"
    "  border-bottom: 1px solid #2a2a2a;"
    "}"
    ".taskbar-btn {"
    "  background: transparent;"
    "  color: #cccccc;"
    "  border: none;"
    "  border-radius: 3px;"
    "  padding: 2px 10px;"
    "  font-size: 13px;"
    "  font-family: monospace;"
    "  min-height: 28px;"
    "}"
    ".taskbar-btn:hover {"
    "  background-color: #2a2a2a;"
    "  color: #ffffff;"
    "}"
    ".taskbar-btn:active {"
    "  background-color: #1e90ff;"
    "  color: #ffffff;"
    "}"
    ".taskbar-btn.active-window {"
    "  background-color: #222244;"
    "  color: #88aaff;"
    "}"
    ".clock-label {"
    "  color: #cccccc;"
    "  font-size: 13px;"
    "  font-family: monospace;"
    "  padding: 0 12px;"
    "}"
    ".separator {"
    "  background-color: #2a2a2a;"
    "  min-width: 1px;"
    "  margin: 4px 4px;"
    "}";

static void on_show_desktop_clicked(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    /* Send signal to compositor to toggle show-desktop */
    system("pkill -USR1 zenithwm 2>/dev/null || true");
    /* Also notify via a simple IPC file */
    const char *home = getenv("HOME");
    if (home) {
        char path[512];
        snprintf(path, sizeof(path), "%s/.cache/zenithwm/show_desktop", home);
        FILE *f = fopen(path, "w");
        if (f) fclose(f);
    }
}

static void on_monitors_clicked(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    if (fork() == 0) {
        setsid();
        execlp("zenith-monitors", "zenith-monitors", NULL);
        _exit(1);
    }
}

void zenith_taskbar_create(GtkApplication *app) {
    /* Apply CSS */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css, TASKBAR_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    /* Create window */
    GtkWindow *win = GTK_WINDOW(gtk_application_window_new(app));
    gtk_widget_add_css_class(GTK_WIDGET(win), "taskbar");
    gtk_window_set_title(win, "ZenithWM Taskbar");
    gtk_window_set_decorated(win, FALSE);
    gtk_widget_set_size_request(GTK_WIDGET(win), -1, 32);

    /* Layer shell setup */
    gtk_layer_init_for_window(win);
    gtk_layer_set_layer(win, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(win, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(win, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(win, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_exclusive_zone(win, 32);
    gtk_layer_set_keyboard_mode(win, GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
    gtk_layer_set_namespace(win, "zenith-taskbar");

    /* Main horizontal box */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(win, hbox);

    /* === LEFT: Start button === */
    GtkWidget *start_btn = gtk_button_new_with_label("⊞  Start");
    gtk_widget_add_css_class(start_btn, "taskbar-btn");
    g_signal_connect(start_btn, "clicked", G_CALLBACK(zenith_startmenu_show), win);
    gtk_box_append(GTK_BOX(hbox), start_btn);

    /* Separator */
    GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(sep1, "separator");
    gtk_box_append(GTK_BOX(hbox), sep1);

    /* === CENTER: Spacer (window list would go here in a full impl) === */
    GtkWidget *spacer = gtk_label_new(NULL);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(hbox), spacer);

    /* === RIGHT: Show desktop === */
    GtkWidget *desktop_btn = gtk_button_new_with_label("⬛");
    gtk_widget_add_css_class(desktop_btn, "taskbar-btn");
    gtk_widget_set_tooltip_text(desktop_btn, "Show Desktop (Super+D)");
    g_signal_connect(desktop_btn, "clicked", G_CALLBACK(on_show_desktop_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox), desktop_btn);

    /* Separator */
    GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(sep2, "separator");
    gtk_box_append(GTK_BOX(hbox), sep2);

    /* Monitor config button */
    GtkWidget *mon_btn = gtk_button_new_with_label("🖥");
    gtk_widget_add_css_class(mon_btn, "taskbar-btn");
    gtk_widget_set_tooltip_text(mon_btn, "Monitor Configuration");
    g_signal_connect(mon_btn, "clicked", G_CALLBACK(on_monitors_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox), mon_btn);

    /* Settings button */
    GtkWidget *settings_btn = gtk_button_new_with_label("⚙");
    gtk_widget_add_css_class(settings_btn, "taskbar-btn");
    gtk_widget_set_tooltip_text(settings_btn, "Settings");
    g_signal_connect(settings_btn, "clicked", G_CALLBACK(zenith_settings_show), win);
    gtk_box_append(GTK_BOX(hbox), settings_btn);

    /* Separator */
    GtkWidget *sep3 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(sep3, "separator");
    gtk_box_append(GTK_BOX(hbox), sep3);

    /* Clock */
    GtkWidget *clock_label = zenith_clock_widget_new();
    gtk_box_append(GTK_BOX(hbox), clock_label);

    gtk_window_present(win);
}