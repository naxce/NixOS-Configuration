#ifndef ZENITH_DESKTOP_H
#define ZENITH_DESKTOP_H

#include <cairo/cairo.h>

struct zenith_server;

#define ZENITH_MAX_ICONS 64
#define ZENITH_ICON_SIZE 48
#define ZENITH_ICON_GRID_X 80
#define ZENITH_ICON_GRID_Y 80

struct zenith_desktop_icon {
    char label[64];
    char exec[256];
    char icon_path[512];
    int x, y;
    int hover;
    int selected;
    cairo_surface_t *icon_surface;
};

struct zenith_desktop {
    struct zenith_server *server;
    int width, height;

    cairo_surface_t *wallpaper;
    cairo_surface_t *surface;
    cairo_t *cr;
    unsigned char *pixels;
    int stride;

    struct zenith_desktop_icon icons[ZENITH_MAX_ICONS];
    int icon_count;

    int needs_redraw;
};

int  zenith_desktop_init(struct zenith_server *server);
void zenith_desktop_destroy(struct zenith_server *server);
void zenith_desktop_render(struct zenith_desktop *desk);
void zenith_desktop_reload_icons(struct zenith_desktop *desk);
void zenith_desktop_click(struct zenith_desktop *desk, int x, int y);

#endif
