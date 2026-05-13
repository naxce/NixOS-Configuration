#ifndef ZENITH_TASKBAR_H
#define ZENITH_TASKBAR_H

#include <cairo/cairo.h>
#include <time.h>

struct zenith_server;

#define TASKBAR_BTN_SETTINGS  0
#define TASKBAR_BTN_START     1
#define TASKBAR_BTN_MINIMIZE  2
#define TASKBAR_BTN_COUNT     3

struct taskbar_button {
    int x, y, w, h;
    char label[32];
    int hover;
    int pressed;
};

struct zenith_taskbar {
    struct zenith_server *server;
    int width, height;

    cairo_surface_t *surface;
    cairo_t *cr;

    unsigned char *pixels;
    int stride;

    struct taskbar_button buttons[TASKBAR_BTN_COUNT];
    int needs_redraw;
    time_t last_time;
    char time_str[32];
    char date_str[32];
};

int  zenith_taskbar_init(struct zenith_server *server);
void zenith_taskbar_destroy(struct zenith_server *server);
void zenith_taskbar_render(struct zenith_taskbar *tb);
void zenith_taskbar_update(struct zenith_taskbar *tb);
void zenith_taskbar_spawn(struct zenith_server *server);
void zenith_taskbar_click(struct zenith_taskbar *tb, int x, int y);

#endif
