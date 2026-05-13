#ifndef ZENITH_SETTINGS_PANEL_H
#define ZENITH_SETTINGS_PANEL_H

#include <cairo/cairo.h>

struct zenith_server;

#define SETTINGS_WIDTH  640
#define SETTINGS_HEIGHT 480

#define SETTINGS_TAB_MONITORS  0
#define SETTINGS_TAB_GENERAL   1
#define SETTINGS_TAB_KEYBINDS  2
#define SETTINGS_TAB_COUNT     3

struct zenith_settings_panel {
    struct zenith_server *server;
    int visible;
    int x, y, w, h;
    int active_tab;

    cairo_surface_t *surface;
    cairo_t *cr;
    unsigned char *pixels;
    int stride;

    int drag_active;
    int drag_ox, drag_oy;

    int needs_redraw;

    int selected_monitor;
    char edit_field[64];
    int edit_field_id;
};

int  zenith_settings_panel_init(struct zenith_server *server);
void zenith_settings_panel_destroy(struct zenith_server *server);
void zenith_settings_panel_toggle(struct zenith_settings_panel *sp);
void zenith_settings_panel_render(struct zenith_settings_panel *sp);
void zenith_settings_panel_click(struct zenith_settings_panel *sp, int x, int y);
void zenith_settings_panel_key(struct zenith_settings_panel *sp, unsigned int key, unsigned int mods);

#endif
