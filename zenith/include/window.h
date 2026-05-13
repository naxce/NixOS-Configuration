#ifndef ZENITH_WINDOW_H
#define ZENITH_WINDOW_H

#include <wayland-server-core.h>

struct zenith_server;

struct zenith_window {
    struct zenith_server *server;
    struct wl_resource *surface;
    struct wl_resource *xdg_surface;
    struct wl_resource *xdg_toplevel;

    int x, y;
    int width, height;
    int minimized;
    int maximized;
    int fullscreen;
    int mapped;

    char title[256];
    char app_id[128];

    struct wl_listener destroy_listener;
    struct wl_listener map_listener;
    struct wl_listener unmap_listener;
    struct wl_listener commit_listener;
};

int  zenith_windows_init(struct zenith_server *server);
void zenith_windows_destroy(struct zenith_server *server);

struct zenith_window *zenith_window_create(struct zenith_server *server);
void zenith_window_destroy(struct zenith_window *win);
void zenith_window_focus(struct zenith_server *server, struct zenith_window *win);
void zenith_window_close(struct zenith_window *win);
void zenith_window_minimize(struct zenith_window *win);
void zenith_window_maximize(struct zenith_window *win);
void zenith_window_fullscreen(struct zenith_window *win, int enable);
void zenith_window_move(struct zenith_window *win, int x, int y);
void zenith_window_resize(struct zenith_window *win, int w, int h);
void zenith_windows_minimize_all(struct zenith_server *server);
void zenith_windows_restore_all(struct zenith_server *server);

#endif
