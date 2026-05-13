#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <cairo/cairo.h>
#include <GLES2/gl2.h>

#include "zenith.h"
#include "taskbar.h"
#include "window.h"
#include "launcher.h"
#include "settings_panel.h"
#include "output.h"

static void draw_taskbar(struct zenith_taskbar *tb) {
    if (!tb->cr) return;
    struct zenith_server *server = tb->server;
    cairo_t *cr = tb->cr;

    unsigned int bg = server->config.taskbar_bg;
    double r = ((bg >> 16) & 0xFF) / 255.0;
    double g = ((bg >> 8)  & 0xFF) / 255.0;
    double b = ((bg)       & 0xFF) / 255.0;
    cairo_set_source_rgb(cr, r, g, b);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_rectangle(cr, 0, tb->height - 1, tb->width, 1);
    cairo_fill(cr);

    unsigned int fg = server->config.taskbar_fg;
    double fr = ((fg >> 16) & 0xFF) / 255.0;
    double fg2 = ((fg >> 8)  & 0xFF) / 255.0;
    double fb = ((fg)        & 0xFF) / 255.0;

    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);

    static const char *labels[] = {"Settings", "Start", "Win-D"};
    static const char *icons[]  = {"\u2699", "\u2630", "\u2A02"};
    for (int i = 0; i < TASKBAR_BTN_COUNT; i++) {
        struct taskbar_button *btn = &tb->buttons[i];
        if (btn->hover) {
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        } else {
            cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
        }
        cairo_rectangle(cr, btn->x + 1, btn->y + 2, btn->w - 2, btn->h - 4);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, fr, fg2, fb);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 11);
        cairo_text_extents_t ext;
        (void)icons;
        cairo_text_extents(cr, labels[i], &ext);
        cairo_move_to(cr, btn->x + (btn->w - ext.width) / 2.0,
                          btn->y + (btn->h + ext.height) / 2.0);
        cairo_show_text(cr, labels[i]);
    }

    int wx = 200;
    cairo_set_font_size(cr, 10);
    for (int i = 0; i < server->window_count && wx < tb->width - 280; i++) {
        struct zenith_window *w = server->windows[i];
        if (!w->mapped) continue;
        int is_focused = (w == server->focused);
        int is_min     = w->minimized;

        if (is_focused)
            cairo_set_source_rgb(cr, 0.18, 0.28, 0.45);
        else if (is_min)
            cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
        else
            cairo_set_source_rgb(cr, 0.14, 0.14, 0.14);

        cairo_rectangle(cr, wx, 3, 140, tb->height - 6);
        cairo_fill(cr);

        if (is_focused) {
            cairo_set_source_rgb(cr, 0.27, 0.53, 1.0);
            cairo_rectangle(cr, wx, tb->height - 3, 140, 3);
            cairo_fill(cr);
        }

        cairo_set_source_rgb(cr, fr, fg2, fb);
        char label[48];
        strncpy(label, w->title, 47);
        label[20] = '\0';
        if (is_min) {
            char tmp[52];
            snprintf(tmp, sizeof(tmp), "[%s]", label);
            strncpy(label, tmp, 47);
        }
        cairo_text_extents_t ext;
        cairo_text_extents(cr, label, &ext);
        cairo_move_to(cr, wx + 6, 3 + (tb->height - 6 + ext.height) / 2.0);
        cairo_show_text(cr, label);
        wx += 144;
    }

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char tstr[32], dstr[32];
    strftime(tstr, sizeof(tstr), "%H:%M:%S", tm);
    strftime(dstr, sizeof(dstr), "%Y-%m-%d", tm);

    cairo_set_source_rgb(cr, fr, fg2, fb);
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);
    cairo_text_extents_t te;
    cairo_text_extents(cr, tstr, &te);
    cairo_move_to(cr, tb->width - te.width - 8, (tb->height + te.height) / 2.0);
    cairo_show_text(cr, tstr);

    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    cairo_text_extents_t de;
    cairo_text_extents(cr, dstr, &de);
    cairo_move_to(cr, tb->width - de.width - 8 - te.width - 16,
                      (tb->height + de.height) / 2.0);
    cairo_show_text(cr, dstr);

    cairo_surface_flush(tb->surface);
}

int zenith_taskbar_init(struct zenith_server *server) {
    if (server->output_count == 0) return 0;
    struct zenith_output *out = server->outputs[0];

    struct zenith_taskbar *tb = calloc(1, sizeof(*tb));
    if (!tb) return 0;
    tb->server = server;
    tb->width  = out->width;
    tb->height = ZENITH_TASKBAR_HEIGHT;

    tb->stride = tb->width * 4;
    tb->pixels = calloc(tb->stride * tb->height, 1);
    if (!tb->pixels) { free(tb); return 0; }

    tb->surface = cairo_image_surface_create_for_data(
        tb->pixels, CAIRO_FORMAT_ARGB32, tb->width, tb->height, tb->stride);
    if (!tb->surface) { free(tb->pixels); free(tb); return 0; }

    tb->cr = cairo_create(tb->surface);
    if (!tb->cr) {
        cairo_surface_destroy(tb->surface);
        free(tb->pixels);
        free(tb);
        return 0;
    }

    int bw = 80;
    int bh = ZENITH_TASKBAR_HEIGHT - 4;
    int by = 2;
    tb->buttons[TASKBAR_BTN_SETTINGS].x = 4;
    tb->buttons[TASKBAR_BTN_SETTINGS].y = by;
    tb->buttons[TASKBAR_BTN_SETTINGS].w = bw;
    tb->buttons[TASKBAR_BTN_SETTINGS].h = bh;
    strncpy(tb->buttons[TASKBAR_BTN_SETTINGS].label, "Settings", 31);

    tb->buttons[TASKBAR_BTN_START].x = 4 + bw + 2;
    tb->buttons[TASKBAR_BTN_START].y = by;
    tb->buttons[TASKBAR_BTN_START].w = bw;
    tb->buttons[TASKBAR_BTN_START].h = bh;
    strncpy(tb->buttons[TASKBAR_BTN_START].label, "Start", 31);

    tb->buttons[TASKBAR_BTN_MINIMIZE].x = 4 + (bw + 2) * 2;
    tb->buttons[TASKBAR_BTN_MINIMIZE].y = by;
    tb->buttons[TASKBAR_BTN_MINIMIZE].w = bw;
    tb->buttons[TASKBAR_BTN_MINIMIZE].h = bh;
    strncpy(tb->buttons[TASKBAR_BTN_MINIMIZE].label, "Win-D", 31);

    tb->needs_redraw = 1;
    server->taskbar = tb;
    draw_taskbar(tb);
    return 1;
}

void zenith_taskbar_render(struct zenith_taskbar *tb) {
    if (!tb->needs_redraw) {
        time_t now = time(NULL);
        if (now == tb->last_time) return;
        tb->last_time = now;
    }
    draw_taskbar(tb);
    tb->needs_redraw = 0;
}

void zenith_taskbar_update(struct zenith_taskbar *tb) {
    if (tb) tb->needs_redraw = 1;
}

void zenith_taskbar_click(struct zenith_taskbar *tb, int x, int y) {
    if (!tb) return;
    struct zenith_server *server = tb->server;

    for (int i = 0; i < TASKBAR_BTN_COUNT; i++) {
        struct taskbar_button *btn = &tb->buttons[i];
        if (x >= btn->x && x < btn->x + btn->w &&
            y >= btn->y && y < btn->y + btn->h) {
            if (i == TASKBAR_BTN_SETTINGS) {
                zenith_settings_panel_toggle(server->settings_panel);
            } else if (i == TASKBAR_BTN_START) {
                zenith_launcher_toggle(server->launcher);
            } else if (i == TASKBAR_BTN_MINIMIZE) {
                zenith_windows_minimize_all(server);
            }
            return;
        }
    }

    int wx = 200;
    for (int i = 0; i < server->window_count && wx < tb->width - 280; i++) {
        struct zenith_window *w = server->windows[i];
        if (!w->mapped) { wx += 0; continue; }
        if (x >= wx && x < wx + 140 && y >= 3 && y < tb->height - 3) {
            if (w->minimized) {
                zenith_window_focus(server, w);
            } else if (w == server->focused) {
                zenith_window_minimize(w);
            } else {
                zenith_window_focus(server, w);
            }
            tb->needs_redraw = 1;
            return;
        }
        wx += 144;
    }
}

void zenith_taskbar_spawn(struct zenith_server *server) {
    (void)server;
}

void zenith_taskbar_destroy(struct zenith_server *server) {
    if (!server->taskbar) return;
    struct zenith_taskbar *tb = server->taskbar;
    if (tb->cr) cairo_destroy(tb->cr);
    if (tb->surface) cairo_surface_destroy(tb->surface);
    free(tb->pixels);
    free(tb);
    server->taskbar = NULL;
}
