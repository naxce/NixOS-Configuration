#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include <cairo/cairo.h>

#include "zenith.h"
#include "settings_panel.h"
#include "config.h"
#include "output.h"
#include "monitor_settings.h"

static void draw_panel_bg(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    cairo_set_source_rgba(cr, 0.05, 0.05, 0.05, 0.97);
    cairo_rectangle(cr, 0, 0, sp->w, sp->h);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.5);
    cairo_rectangle(cr, 0, 0, sp->w, 1);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, 0, 1, sp->h);
    cairo_fill(cr);
    cairo_rectangle(cr, sp->w - 1, 0, 1, sp->h);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, sp->h - 1, sp->w, 1);
    cairo_fill(cr);
}

static void draw_tab_bar(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    static const char *tabs[] = {"Monitors", "General", "Keybinds"};
    int tw = sp->w / SETTINGS_TAB_COUNT;
    for (int i = 0; i < SETTINGS_TAB_COUNT; i++) {
        if (i == sp->active_tab) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.2);
        } else {
            cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
        }
        cairo_rectangle(cr, i * tw, 28, tw, 24);
        cairo_fill(cr);

        if (i == sp->active_tab) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 1.0);
            cairo_rectangle(cr, i * tw, 51, tw, 2);
            cairo_fill(cr);
        }

        cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 11);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, tabs[i], &ext);
        cairo_move_to(cr, i * tw + (tw - ext.width) / 2.0, 44);
        cairo_show_text(cr, tabs[i]);
    }
    cairo_set_source_rgba(cr, 0.15, 0.15, 0.15, 1.0);
    cairo_rectangle(cr, 0, 53, sp->w, 1);
    cairo_fill(cr);
}

static void draw_title_bar(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    cairo_set_source_rgba(cr, 0.08, 0.08, 0.08, 1.0);
    cairo_rectangle(cr, 0, 0, sp->w, 28);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 10, 18);
    cairo_show_text(cr, "Zenith Settings");

    cairo_set_source_rgba(cr, 0.8, 0.2, 0.2, 0.8);
    cairo_rectangle(cr, sp->w - 24, 4, 20, 20);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, sp->w - 18, 17);
    cairo_show_text(cr, "X");
}

static void draw_monitors_tab(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    struct zenith_server *server = sp->server;
    int y = 62;

    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_move_to(cr, 10, y + 12);
    cairo_show_text(cr, "Connected outputs:");
    y += 22;

    for (int i = 0; i < server->output_count; i++) {
        struct zenith_output *out = server->outputs[i];
        int selected = (i == sp->selected_monitor);

        if (selected) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.15);
        } else {
            cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
        }
        cairo_rectangle(cr, 8, y, sp->w - 16, 58);
        cairo_fill(cr);

        if (selected) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.6);
            cairo_rectangle(cr, 8, y, 2, 58);
            cairo_fill(cr);
        }

        char buf[256];
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 11);
        snprintf(buf, sizeof(buf), "Output %d: %s", i, out->name);
        cairo_move_to(cr, 14, y + 14);
        cairo_show_text(cr, buf);

        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
        snprintf(buf, sizeof(buf), "  %dx%d @ %dHz  pos: %d,%d  scale: %.1f  %s",
            out->width, out->height, out->refresh,
            out->x, out->y, out->scale,
            out->enabled ? "enabled" : "DISABLED");
        cairo_move_to(cr, 14, y + 28);
        cairo_show_text(cr, buf);

        if (selected) {
            snprintf(buf, sizeof(buf), "  [Enter] Apply  [E] Toggle enable  [+/-] Scale  [Arrow] Position");
            cairo_set_source_rgb(cr, 0.4, 0.6, 0.9);
            cairo_move_to(cr, 14, y + 44);
            cairo_show_text(cr, buf);
        }
        y += 64;
    }

    if (server->output_count == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, 14, y + 14);
        cairo_show_text(cr, "No outputs detected.");
    }

    y += 10;
    cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.8);
    cairo_rectangle(cr, 8, y, 120, 24);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_move_to(cr, 14, y + 16);
    cairo_show_text(cr, "Save & Apply");
}

static void draw_general_tab(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    struct zenith_server *server = sp->server;
    int y = 66;
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 11);

    struct { const char *label; const char *value; } items[] = {
        {"Terminal",   server->config.terminal},
        {"Wallpaper",  server->config.wallpaper[0] ? server->config.wallpaper : "(none)"},
        {"Border px",  NULL},
        {"Gaps",       NULL},
        {NULL, NULL}
    };
    char ibuf1[32], ibuf2[32];
    snprintf(ibuf1, sizeof(ibuf1), "%d", server->config.border_width);
    snprintf(ibuf2, sizeof(ibuf2), "%d", server->config.gaps);
    items[2].value = ibuf1;
    items[3].value = ibuf2;

    for (int i = 0; items[i].label; i++) {
        cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
        cairo_move_to(cr, 12, y);
        cairo_show_text(cr, items[i].label);

        cairo_set_source_rgba(cr, 0.12, 0.12, 0.12, 1.0);
        cairo_rectangle(cr, 140, y - 12, sp->w - 160, 18);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
        cairo_move_to(cr, 144, y);
        cairo_show_text(cr, items[i].value);
        y += 28;
    }

    y += 10;
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_font_size(cr, 10);
    cairo_move_to(cr, 12, y);
    cairo_show_text(cr, "Edit ~/.config/zenith/zenith.conf for full config.");
    y += 18;
    cairo_move_to(cr, 12, y);
    cairo_show_text(cr, "Super+Shift+R to reload config.");
}

static void draw_keybinds_tab(struct zenith_settings_panel *sp) {
    cairo_t *cr = sp->cr;
    int y = 66;
    static const char *binds[][2] = {
        {"Super + Enter",       "Open terminal"},
        {"Super + Space",       "Toggle launcher / start menu"},
        {"Super + Shift + Q",   "Close focused window"},
        {"Super + H",           "Minimize focused window"},
        {"Super + M",           "Maximize focused window"},
        {"Super + D",           "Minimize / restore all windows"},
        {"Alt + Tab",           "Focus next window"},
        {"Alt + F4",            "Close window"},
        {"F11",                 "Toggle fullscreen"},
        {"Super + Shift + R",   "Reload config"},
        {"Ctrl + Super + Q",    "Quit Zenith"},
        {NULL, NULL}
    };
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    for (int i = 0; binds[i][0]; i++) {
        cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.9);
        cairo_move_to(cr, 12, y);
        cairo_show_text(cr, binds[i][0]);

        cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
        cairo_move_to(cr, 230, y);
        cairo_show_text(cr, binds[i][1]);
        y += 20;
        if (y > sp->h - 20) break;
    }
}

static void redraw(struct zenith_settings_panel *sp) {
    if (!sp->cr) return;
    cairo_t *cr = sp->cr;
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    draw_panel_bg(sp);
    draw_title_bar(sp);
    draw_tab_bar(sp);

    if (sp->active_tab == SETTINGS_TAB_MONITORS)
        draw_monitors_tab(sp);
    else if (sp->active_tab == SETTINGS_TAB_GENERAL)
        draw_general_tab(sp);
    else if (sp->active_tab == SETTINGS_TAB_KEYBINDS)
        draw_keybinds_tab(sp);

    cairo_surface_flush(sp->surface);
    sp->needs_redraw = 0;
}

int zenith_settings_panel_init(struct zenith_server *server) {
    struct zenith_settings_panel *sp = calloc(1, sizeof(*sp));
    if (!sp) return 0;
    sp->server = server;
    sp->w = SETTINGS_WIDTH;
    sp->h = SETTINGS_HEIGHT;

    int ow = 800, oh = 600;
    if (server->output_count > 0) {
        ow = server->outputs[0]->width;
        oh = server->outputs[0]->height;
    }
    sp->x = (ow - sp->w) / 2;
    sp->y = (oh - sp->h) / 2;
    sp->active_tab = SETTINGS_TAB_MONITORS;
    sp->visible = 0;
    sp->needs_redraw = 1;

    sp->stride = sp->w * 4;
    sp->pixels = calloc(sp->stride * sp->h, 1);
    sp->surface = cairo_image_surface_create_for_data(
        sp->pixels, CAIRO_FORMAT_ARGB32, sp->w, sp->h, sp->stride);
    sp->cr = cairo_create(sp->surface);

    server->settings_panel = sp;
    return 1;
}

void zenith_settings_panel_toggle(struct zenith_settings_panel *sp) {
    if (!sp) return;
    sp->visible = !sp->visible;
    if (sp->visible) sp->needs_redraw = 1;
}

void zenith_settings_panel_render(struct zenith_settings_panel *sp) {
    if (!sp->visible) return;
    if (sp->needs_redraw) redraw(sp);
}

void zenith_settings_panel_click(struct zenith_settings_panel *sp, int x, int y) {
    if (!sp->visible) return;
    int rx = x - sp->x, ry = y - sp->y;
    if (rx < 0 || rx >= sp->w || ry < 0 || ry >= sp->h) {
        sp->visible = 0; return;
    }

    if (ry >= 4 && ry <= 24 && rx >= sp->w - 24 && rx <= sp->w - 4) {
        sp->visible = 0; return;
    }

    if (ry >= 0 && ry <= 28) {
        sp->drag_active = 1;
        sp->drag_ox = rx; sp->drag_oy = ry;
        return;
    }

    if (ry >= 28 && ry <= 53) {
        int tw = sp->w / SETTINGS_TAB_COUNT;
        int tab = rx / tw;
        if (tab >= 0 && tab < SETTINGS_TAB_COUNT) {
            sp->active_tab = tab;
            sp->needs_redraw = 1;
        }
        return;
    }

    if (sp->active_tab == SETTINGS_TAB_MONITORS) {
        struct zenith_server *server = sp->server;
        int y_off = 62 + 22;
        for (int i = 0; i < server->output_count; i++) {
            if (ry >= y_off && ry < y_off + 58) {
                sp->selected_monitor = i;
                sp->needs_redraw = 1;
                break;
            }
            y_off += 64;
        }
        int save_y = 62 + 22 + server->output_count * 64 + 10;
        if (ry >= save_y && ry < save_y + 24 && rx >= 8 && rx < 128) {
            zenith_config_save(&server->config);
            zenith_monitor_settings_apply(server);
            sp->needs_redraw = 1;
        }
    }
}

void zenith_settings_panel_key(struct zenith_settings_panel *sp, unsigned int key, unsigned int mods) {
    (void)mods;
    if (!sp->visible) return;
    if (key == KEY_ESC) { sp->visible = 0; return; }

    if (sp->active_tab == SETTINGS_TAB_MONITORS) {
        struct zenith_server *server = sp->server;
        int i = sp->selected_monitor;
        if (i < 0 || i >= server->output_count) return;
        struct zenith_output *out = server->outputs[i];
        if (key == KEY_E) {
            out->enabled = !out->enabled;
            sp->needs_redraw = 1;
        } else if (key == KEY_EQUAL) {
            out->scale += 0.25f;
            if (out->scale > 4.0f) out->scale = 4.0f;
            sp->needs_redraw = 1;
        } else if (key == KEY_MINUS) {
            out->scale -= 0.25f;
            if (out->scale < 0.25f) out->scale = 0.25f;
            sp->needs_redraw = 1;
        } else if (key == KEY_LEFT) {
            out->x -= 10; sp->needs_redraw = 1;
        } else if (key == KEY_RIGHT) {
            out->x += 10; sp->needs_redraw = 1;
        } else if (key == KEY_UP) {
            out->y -= 10; sp->needs_redraw = 1;
        } else if (key == KEY_DOWN) {
            out->y += 10; sp->needs_redraw = 1;
        } else if (key == KEY_ENTER) {
            zenith_config_save(&server->config);
            zenith_monitor_settings_apply(server);
            sp->needs_redraw = 1;
        }
    }
}

void zenith_settings_panel_destroy(struct zenith_server *server) {
    if (!server->settings_panel) return;
    struct zenith_settings_panel *sp = server->settings_panel;
    if (sp->cr) cairo_destroy(sp->cr);
    if (sp->surface) cairo_surface_destroy(sp->surface);
    free(sp->pixels);
    free(sp);
    server->settings_panel = NULL;
}
