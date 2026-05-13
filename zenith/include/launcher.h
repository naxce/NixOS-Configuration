#ifndef ZENITH_LAUNCHER_H
#define ZENITH_LAUNCHER_H

#include <cairo/cairo.h>

struct zenith_server;

#define ZENITH_LAUNCHER_MAX_RESULTS 20
#define ZENITH_LAUNCHER_WIDTH  500
#define ZENITH_LAUNCHER_HEIGHT 400

struct launcher_entry {
    char name[128];
    char exec[256];
    char icon[512];
    char comment[256];
};

struct zenith_launcher {
    struct zenith_server *server;
    int visible;

    int x, y, w, h;

    cairo_surface_t *surface;
    cairo_t *cr;
    unsigned char *pixels;
    int stride;

    char query[256];
    int query_len;

    struct launcher_entry all_apps[512];
    int app_count;

    struct launcher_entry results[ZENITH_LAUNCHER_MAX_RESULTS];
    int result_count;
    int selected;

    int needs_redraw;
};

int  zenith_launcher_init(struct zenith_server *server);
void zenith_launcher_destroy(struct zenith_server *server);
void zenith_launcher_toggle(struct zenith_launcher *launcher);
void zenith_launcher_render(struct zenith_launcher *launcher);
void zenith_launcher_key(struct zenith_launcher *launcher, unsigned int key, unsigned int mods);
void zenith_launcher_click(struct zenith_launcher *launcher, int x, int y);
void zenith_launcher_scan_apps(struct zenith_launcher *launcher);

#endif
