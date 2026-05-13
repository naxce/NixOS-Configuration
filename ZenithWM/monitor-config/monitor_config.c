/*
 * monitor_config.c
 * GTK4 UI for monitor management. Reads current outputs via wlr-output-management
 * protocol, shows controls per monitor, writes monitors.conf on Apply.
 */

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "monitor_config.h"

/* We use the wlr-output-management protocol for live changes.
   For simplicity this implementation reads xrandr-style info via
   /sys/class/drm and wl_output, then writes the config file.
   A full implementation would use zwlr_output_manager_v1. */

#define MAX_OUTPUTS 8
#define MAX_MODES   64

typedef struct {
    int width, height, refresh; /* refresh in mHz */
} OutputMode;

typedef struct {
    char name[64];
    char make[128];
    char model[128];
    int current_width, current_height, current_refresh;
    int pos_x, pos_y;
    float scale;
    int transform; /* 0=normal,90,180,270 */
    bool adaptive_sync;
    bool enabled;
    OutputMode modes[MAX_MODES];
    int n_modes;
} OutputInfo;

static OutputInfo outputs[MAX_OUTPUTS];
static int n_outputs = 0;

/* ------------------------------------------------------------------ */
/*  Load saved config                                                   */
/* ------------------------------------------------------------------ */

static void load_saved(void) {
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/monitors.conf", home);
    FILE *f = fopen(path, "r");
    if (!f) return;

    int cur = -1;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "[output]") == 0) { cur++; if (cur >= n_outputs) break; continue; }
        if (cur < 0) continue;
        char key[64], val[192];
        if (sscanf(line, " %63[^= ] = %191[^\n]", key, val) != 2) continue;

        /* Match output by name */
        if (strcmp(key, "name") == 0) {
            cur = -1;
            for (int i = 0; i < n_outputs; i++) {
                if (strcmp(outputs[i].name, val) == 0) { cur = i; break; }
            }
            continue;
        }
        if (cur < 0) continue;
        if (strcmp(key, "width") == 0)  outputs[cur].current_width  = atoi(val);
        else if (strcmp(key, "height") == 0) outputs[cur].current_height = atoi(val);
        else if (strcmp(key, "refresh") == 0) outputs[cur].current_refresh = atoi(val);
        else if (strcmp(key, "x") == 0)      outputs[cur].pos_x = atoi(val);
        else if (strcmp(key, "y") == 0)      outputs[cur].pos_y = atoi(val);
        else if (strcmp(key, "scale") == 0)  outputs[cur].scale = atof(val);
        else if (strcmp(key, "enabled") == 0) outputs[cur].enabled = (strcmp(val,"true")==0);
        else if (strcmp(key, "adaptive_sync") == 0)
            outputs[cur].adaptive_sync = (strcmp(val,"true")==0);
        else if (strcmp(key, "transform") == 0) {
            if (strcmp(val,"90")==0) outputs[cur].transform=1;
            else if (strcmp(val,"180")==0) outputs[cur].transform=2;
            else if (strcmp(val,"270")==0) outputs[cur].transform=3;
            else outputs[cur].transform=0;
        }
    }
    fclose(f);
}

/* ------------------------------------------------------------------ */
/*  Discover outputs via wl_output (minimal, gets name + modes)        */
/* ------------------------------------------------------------------ */

static struct wl_display *wl_dpy = NULL;
static struct wl_registry *wl_reg = NULL;

static void output_handle_geometry(void *data, struct wl_output *wl_output,
    int32_t x, int32_t y, int32_t phy_w, int32_t phy_h,
    int32_t subpixel, const char *make, const char *model, int32_t transform) {
    (void)wl_output; (void)phy_w; (void)phy_h; (void)subpixel;
    OutputInfo *o = data;
    o->pos_x = x; o->pos_y = y;
    strncpy(o->make, make, sizeof(o->make)-1);
    strncpy(o->model, model, sizeof(o->model)-1);
    o->transform = transform;
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
    uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    (void)wl_output;
    OutputInfo *o = data;
    if (o->n_modes < MAX_MODES) {
        o->modes[o->n_modes++] = (OutputMode){width, height, refresh};
    }
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        o->current_width = width;
        o->current_height = height;
        o->current_refresh = refresh;
    }
}

static void output_handle_done(void *data, struct wl_output *wl_output) {
    (void)data; (void)wl_output;
}

static void output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
    (void)wl_output;
    OutputInfo *o = data;
    o->scale = (float)factor;
}

static void output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
    (void)wl_output;
    OutputInfo *o = data;
    strncpy(o->name, name, sizeof(o->name)-1);
}

static void output_handle_description(void *data, struct wl_output *wl_output, const char *desc) {
    (void)data; (void)wl_output; (void)desc;
}

static const struct wl_output_listener output_listener = {
    .geometry    = output_handle_geometry,
    .mode        = output_handle_mode,
    .done        = output_handle_done,
    .scale       = output_handle_scale,
    .name        = output_handle_name,
    .description = output_handle_description,
};

static void registry_global(void *data, struct wl_registry *reg,
    uint32_t name, const char *iface, uint32_t version) {
    (void)data;
    if (strcmp(iface, "wl_output") == 0 && n_outputs < MAX_OUTPUTS) {
        OutputInfo *o = &outputs[n_outputs];
        memset(o, 0, sizeof(*o));
        o->scale = 1.0f;
        o->enabled = true;
        struct wl_output *wl_out = wl_registry_bind(reg, name,
            &wl_output_interface, version < 4 ? version : 4);
        wl_output_add_listener(wl_out, &output_listener, o);
        n_outputs++;
    }
}

static void registry_global_remove(void *data, struct wl_registry *reg, uint32_t name) {
    (void)data; (void)reg; (void)name;
}

static const struct wl_registry_listener reg_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

static void discover_outputs(void) {
    n_outputs = 0;
    const char *display = getenv("WAYLAND_DISPLAY");
    if (!display) display = "wayland-0";
    wl_dpy = wl_display_connect(display);
    if (!wl_dpy) {
        /* Fallback: create a dummy entry */
        strcpy(outputs[0].name, "Unknown");
        outputs[0].scale = 1.0f;
        outputs[0].enabled = true;
        n_outputs = 1;
        return;
    }
    wl_reg = wl_display_get_registry(wl_dpy);
    wl_registry_add_listener(wl_reg, &reg_listener, NULL);
    wl_display_roundtrip(wl_dpy);
    wl_display_roundtrip(wl_dpy); /* second for mode/done events */
}

/* ------------------------------------------------------------------ */
/*  Save config                                                         */
/* ------------------------------------------------------------------ */

static void save_config(void) {
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/monitors.conf", home);
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "# ZenithWM Monitor Configuration\n"
               "# Generated by zenith-monitors — edit carefully\n\n");

    for (int i = 0; i < n_outputs; i++) {
        OutputInfo *o = &outputs[i];
        fprintf(f, "[output]\n");
        fprintf(f, "name = %s\n", o->name);
        fprintf(f, "enabled = %s\n", o->enabled ? "true" : "false");
        fprintf(f, "width = %d\n", o->current_width);
        fprintf(f, "height = %d\n", o->current_height);
        fprintf(f, "refresh = %d\n", o->current_refresh);
        fprintf(f, "x = %d\n", o->pos_x);
        fprintf(f, "y = %d\n", o->pos_y);
        fprintf(f, "scale = %.1f\n", o->scale);
        const char *transforms[] = {"normal","90","180","270"};
        fprintf(f, "transform = %s\n", transforms[o->transform % 4]);
        fprintf(f, "adaptive_sync = %s\n\n", o->adaptive_sync ? "true" : "false");
    }
    fclose(f);

    /* Signal compositor to re-apply */
    system("pkill -USR1 zenithwm 2>/dev/null || true");
}

/* ------------------------------------------------------------------ */
/*  Per-output widget row                                               */
/* ------------------------------------------------------------------ */

typedef struct {
    int idx;
    GtkDropDown *mode_dd;
    GtkSpinButton *pos_x, *pos_y;
    GtkDropDown *scale_dd;
    GtkDropDown *rot_dd;
    GtkSwitch *enabled_sw;
    GtkSwitch *vrr_sw;
} OutputWidgets;

static void on_apply(GtkButton *btn, gpointer data) {
    (void)btn;
    GArray *widgets_arr = data;
    for (guint i = 0; i < widgets_arr->len; i++) {
        OutputWidgets *w = &g_array_index(widgets_arr, OutputWidgets, i);
        OutputInfo *o = &outputs[w->idx];
        o->enabled      = gtk_switch_get_active(w->enabled_sw);
        o->adaptive_sync = gtk_switch_get_active(w->vrr_sw);
        o->pos_x        = (int)gtk_spin_button_get_value(w->pos_x);
        o->pos_y        = (int)gtk_spin_button_get_value(w->pos_y);
        o->transform    = (int)gtk_drop_down_get_selected(w->rot_dd);

        /* Scale */
        static const float scales[] = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
        guint si = gtk_drop_down_get_selected(w->scale_dd);
        if (si < G_N_ELEMENTS(scales)) o->scale = scales[si];

        /* Mode */
        guint mi = gtk_drop_down_get_selected(w->mode_dd);
        if ((int)mi < o->n_modes) {
            o->current_width   = o->modes[mi].width;
            o->current_height  = o->modes[mi].height;
            o->current_refresh = o->modes[mi].refresh;
        }
    }
    save_config();
}

static GtkWidget *build_output_widget(int idx, GArray *widgets_arr) {
    OutputInfo *o = &outputs[idx];

    GtkWidget *frame = gtk_frame_new(NULL);
    char title[128];
    snprintf(title, sizeof(title), "%s  (%s %s)",
        o->name, o->make, o->model);
    gtk_frame_set_label(GTK_FRAME(frame), title);
    gtk_widget_set_margin_start(frame, 8);
    gtk_widget_set_margin_end(frame, 8);
    gtk_widget_set_margin_top(frame, 6);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 8);
    gtk_widget_set_margin_bottom(grid, 8);
    gtk_frame_set_child(GTK_FRAME(frame), grid);

    int row = 0;
    OutputWidgets w = {0};
    w.idx = idx;

#define ADD_ROW(label_str, widget) do { \
    GtkWidget *lbl = gtk_label_new(label_str); \
    gtk_widget_set_halign(lbl, GTK_ALIGN_START); \
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 1, 1); \
    gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1); \
    row++; \
} while(0)

    /* Enabled */
    w.enabled_sw = GTK_SWITCH(gtk_switch_new());
    gtk_switch_set_active(w.enabled_sw, o->enabled);
    ADD_ROW("Enabled", GTK_WIDGET(w.enabled_sw));

    /* Resolution + refresh dropdown */
    GtkStringList *mode_list = gtk_string_list_new(NULL);
    int current_mi = 0;
    for (int m = 0; m < o->n_modes; m++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%dx%d @ %d Hz",
            o->modes[m].width, o->modes[m].height, o->modes[m].refresh / 1000);
        gtk_string_list_append(mode_list, buf);
        if (o->modes[m].width  == o->current_width &&
            o->modes[m].height == o->current_height &&
            o->modes[m].refresh == o->current_refresh)
            current_mi = m;
    }
    if (o->n_modes == 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%dx%d @ %d Hz",
            o->current_width, o->current_height, o->current_refresh/1000);
        gtk_string_list_append(mode_list, buf);
    }
    w.mode_dd = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(mode_list), NULL));
    gtk_drop_down_set_selected(w.mode_dd, current_mi);
    ADD_ROW("Resolution & Refresh", GTK_WIDGET(w.mode_dd));

    /* Position */
    w.pos_x = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(-8192, 8192, 1));
    gtk_spin_button_set_value(w.pos_x, o->pos_x);
    ADD_ROW("Position X", GTK_WIDGET(w.pos_x));

    w.pos_y = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(-8192, 8192, 1));
    gtk_spin_button_set_value(w.pos_y, o->pos_y);
    ADD_ROW("Position Y", GTK_WIDGET(w.pos_y));

    /* Scale */
    GtkStringList *scale_list = gtk_string_list_new(NULL);
    const char *scale_opts[] = {"1.0x", "1.25x", "1.5x", "1.75x", "2.0x", NULL};
    const float scale_vals[] = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
    int scale_idx = 0;
    for (int s = 0; scale_opts[s]; s++) {
        gtk_string_list_append(scale_list, scale_opts[s]);
        if (scale_vals[s] == o->scale) scale_idx = s;
    }
    w.scale_dd = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(scale_list), NULL));
    gtk_drop_down_set_selected(w.scale_dd, scale_idx);
    ADD_ROW("Scale", GTK_WIDGET(w.scale_dd));

    /* Rotation */
    GtkStringList *rot_list = gtk_string_list_new(NULL);
    const char *rots[] = {"Normal (0°)", "90°", "180°", "270°", NULL};
    for (int r = 0; rots[r]; r++) gtk_string_list_append(rot_list, rots[r]);
    w.rot_dd = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(rot_list), NULL));
    gtk_drop_down_set_selected(w.rot_dd, o->transform % 4);
    ADD_ROW("Rotation", GTK_WIDGET(w.rot_dd));

    /* VRR */
    w.vrr_sw = GTK_SWITCH(gtk_switch_new());
    gtk_switch_set_active(w.vrr_sw, o->adaptive_sync);
    ADD_ROW("Adaptive Sync (VRR)", GTK_WIDGET(w.vrr_sw));

#undef ADD_ROW

    g_array_append_val(widgets_arr, w);
    return frame;
}

/* ------------------------------------------------------------------ */
/*  Main window                                                         */
/* ------------------------------------------------------------------ */

void zenith_monitor_config_window(GtkApplication *app) {
    discover_outputs();
    load_saved();

    GtkWindow *win = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(win, "ZenithWM — Monitor Configuration");
    gtk_window_set_default_size(win, 480, 560);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header),
        "<big><b>Monitor Configuration</b></big>\n"
        "<small>Changes apply immediately after clicking Apply.</small>");
    gtk_widget_set_margin_start(header, 12);
    gtk_widget_set_margin_top(header, 12);
    gtk_widget_set_margin_bottom(header, 6);
    gtk_widget_set_halign(header, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), header);
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);

    GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GArray *widgets_arr = g_array_new(FALSE, TRUE, sizeof(OutputWidgets));

    if (n_outputs == 0) {
        gtk_box_append(GTK_BOX(inner),
            gtk_label_new("No outputs detected. Is the compositor running?"));
    }
    for (int i = 0; i < n_outputs; i++) {
        gtk_box_append(GTK_BOX(inner), build_output_widget(i, widgets_arr));
    }

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), inner);
    gtk_box_append(GTK_BOX(vbox), scroll);

    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_END);
    gtk_widget_set_margin_end(btn_row, 12);
    gtk_widget_set_margin_top(btn_row, 8);
    gtk_widget_set_margin_bottom(btn_row, 8);

    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btn_row), close_btn);

    GtkWidget *apply_btn = gtk_button_new_with_label("Apply & Save");
    gtk_widget_add_css_class(apply_btn, "suggested-action");
    g_signal_connect(apply_btn, "clicked", G_CALLBACK(on_apply), widgets_arr);
    gtk_box_append(GTK_BOX(btn_row), apply_btn);

    gtk_box_append(GTK_BOX(vbox), btn_row);
    gtk_window_set_child(win, vbox);
    gtk_window_present(win);
}