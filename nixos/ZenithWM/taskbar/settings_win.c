/*
 * settings_win.c - Simple settings dialog
 * Edits ~/.config/zenithwm/zenithwm.conf and keys.conf in-place.
 */

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "settings_win.h"

typedef struct {
    GtkSwitch *adaptive_sync;
    GtkSwitch *vsync;
    GtkSwitch *xwayland;
    GtkSwitch *no_animations;
    GtkEntry  *terminal;
    GtkSpinButton *border_width;
} SettingsWidgets;

static void read_config(SettingsWidgets *w) {
    bool adaptive_sync = true, vsync = false, xwayland = true, no_anim = true;
    int border = 1;
    char terminal[256] = "foot";

    const char *home = getenv("HOME");
    if (!home) goto apply;
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/zenithwm.conf", home);
    FILE *f = fopen(path, "r");
    if (!f) goto apply;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], val[192];
        if (sscanf(line, " %63[^= ] = %191[^\n]", key, val) != 2) continue;
        if (strcmp(key, "adaptive_sync") == 0) adaptive_sync = (strcmp(val,"true")==0);
        else if (strcmp(key, "vsync") == 0)    vsync = (strcmp(val,"true")==0);
        else if (strcmp(key, "xwayland") == 0) xwayland = (strcmp(val,"true")==0);
        else if (strcmp(key, "no_animations") == 0) no_anim = (strcmp(val,"true")==0);
        else if (strcmp(key, "border_width") == 0)  border = atoi(val);
        else if (strcmp(key, "terminal") == 0) strncpy(terminal, val, sizeof(terminal)-1);
    }
    fclose(f);

apply:
    gtk_switch_set_active(w->adaptive_sync,   adaptive_sync);
    gtk_switch_set_active(w->vsync,           vsync);
    gtk_switch_set_active(w->xwayland,        xwayland);
    gtk_switch_set_active(w->no_animations,   no_anim);
    gtk_editable_set_text(GTK_EDITABLE(w->terminal), terminal);
    gtk_spin_button_set_value(w->border_width, border);
}

static void write_config(SettingsWidgets *w) {
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/zenithwm.conf", home);
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "[compositor]\n");
    fprintf(f, "adaptive_sync = %s\n", gtk_switch_get_active(w->adaptive_sync) ? "true":"false");
    fprintf(f, "vsync = %s\n",         gtk_switch_get_active(w->vsync)         ? "true":"false");
    fprintf(f, "xwayland = %s\n",      gtk_switch_get_active(w->xwayland)      ? "true":"false");
    fprintf(f, "border_width = %d\n",  (int)gtk_spin_button_get_value(w->border_width));
    fprintf(f, "\n[performance]\n");
    fprintf(f, "no_animations = %s\n", gtk_switch_get_active(w->no_animations) ? "true":"false");
    fprintf(f, "\n[apps]\n");
    fprintf(f, "terminal = %s\n", gtk_editable_get_text(GTK_EDITABLE(w->terminal)));
    fclose(f);

    /* Signal compositor to reload */
    system("pkill -USR1 zenithwm 2>/dev/null || true");
    /* Reload keybinds too */
    system("pkill -USR1 zenith-taskbar 2>/dev/null || true");
}

static GtkWidget *make_row(const char *label, GtkWidget *control) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(row, 12);
    gtk_widget_set_margin_end(row, 12);
    gtk_widget_set_margin_top(row, 6);
    gtk_widget_set_margin_bottom(row, 6);
    GtkWidget *lbl = gtk_label_new(label);
    gtk_widget_set_hexpand(lbl, TRUE);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row), lbl);
    gtk_box_append(GTK_BOX(row), control);
    return row;
}

void zenith_settings_show(GtkButton *btn, gpointer parent_win) {
    (void)btn;
    GtkWindow *dialog = GTK_WINDOW(gtk_window_new());
    gtk_window_set_title(dialog, "ZenithWM Settings");
    gtk_window_set_transient_for(dialog, GTK_WINDOW(parent_win));
    gtk_window_set_modal(dialog, TRUE);
    gtk_window_set_default_size(dialog, 360, 400);
    gtk_window_set_resizable(dialog, FALSE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Section: Compositor */
    GtkWidget *comp_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(comp_lbl), "<b>Compositor</b>");
    gtk_widget_set_margin_start(comp_lbl, 12);
    gtk_widget_set_margin_top(comp_lbl, 12);
    gtk_widget_set_halign(comp_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), comp_lbl);

    SettingsWidgets *w = g_new0(SettingsWidgets, 1);

    w->adaptive_sync = GTK_SWITCH(gtk_switch_new());
    gtk_box_append(GTK_BOX(vbox), make_row("Adaptive Sync (VRR/FreeSync)", GTK_WIDGET(w->adaptive_sync)));

    w->vsync = GTK_SWITCH(gtk_switch_new());
    gtk_box_append(GTK_BOX(vbox), make_row("VSync (disable for lower latency)", GTK_WIDGET(w->vsync)));

    w->xwayland = GTK_SWITCH(gtk_switch_new());
    gtk_box_append(GTK_BOX(vbox), make_row("XWayland (legacy games)", GTK_WIDGET(w->xwayland)));

    w->border_width = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 8, 1));
    gtk_box_append(GTK_BOX(vbox), make_row("Window border (px)", GTK_WIDGET(w->border_width)));

    /* Section: Performance */
    GtkWidget *perf_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(perf_lbl), "<b>Performance</b>");
    gtk_widget_set_margin_start(perf_lbl, 12);
    gtk_widget_set_margin_top(perf_lbl, 8);
    gtk_widget_set_halign(perf_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), perf_lbl);

    w->no_animations = GTK_SWITCH(gtk_switch_new());
    gtk_box_append(GTK_BOX(vbox), make_row("Disable animations", GTK_WIDGET(w->no_animations)));

    /* Section: Apps */
    GtkWidget *apps_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(apps_lbl), "<b>Applications</b>");
    gtk_widget_set_margin_start(apps_lbl, 12);
    gtk_widget_set_margin_top(apps_lbl, 8);
    gtk_widget_set_halign(apps_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), apps_lbl);

    w->terminal = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(w->terminal, "e.g. foot");
    gtk_box_append(GTK_BOX(vbox), make_row("Terminal emulator", GTK_WIDGET(w->terminal)));

    /* Monitor config shortcut */
    GtkWidget *mon_btn = gtk_button_new_with_label("Open Monitor Configuration…");
    gtk_widget_set_margin_start(mon_btn, 12);
    gtk_widget_set_margin_end(mon_btn, 12);
    gtk_widget_set_margin_top(mon_btn, 8);
    g_signal_connect_swapped(mon_btn, "clicked",
        G_CALLBACK(system), "zenith-monitors &");
    gtk_box_append(GTK_BOX(vbox), mon_btn);

    /* Spacer */
    GtkWidget *sp = gtk_label_new(NULL);
    gtk_widget_set_vexpand(sp, TRUE);
    gtk_box_append(GTK_BOX(vbox), sp);

    /* Buttons */
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_END);
    gtk_widget_set_margin_start(btn_row, 12);
    gtk_widget_set_margin_end(btn_row, 12);
    gtk_widget_set_margin_top(btn_row, 8);
    gtk_widget_set_margin_bottom(btn_row, 8);

    GtkWidget *cancel = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_row), cancel);

    GtkWidget *apply = gtk_button_new_with_label("Apply & Save");
    gtk_widget_add_css_class(apply, "suggested-action");
    g_signal_connect_swapped(apply, "clicked", G_CALLBACK(write_config), w);
    g_signal_connect_swapped(apply, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_row), apply);

    gtk_box_append(GTK_BOX(vbox), btn_row);

    gtk_window_set_child(dialog, vbox);
    read_config(w);
    gtk_window_present(dialog);
}