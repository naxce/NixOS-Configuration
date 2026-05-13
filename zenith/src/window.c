#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include "zenith.h"
#include "output.h"
#include "window.h"
#include "taskbar.h"

static void xdg_toplevel_handle_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_window *win = wl_container_of(listener, win, destroy_listener);
    zenith_window_destroy(win);
}

int zenith_windows_init(struct zenith_server *server) {
    server->window_count = 0;
    server->focused = NULL;
    return 1;
}

void zenith_windows_destroy(struct zenith_server *server) {
    for (int i = 0; i < server->window_count; i++) {
        if (server->windows[i]) {
            free(server->windows[i]);
            server->windows[i] = NULL;
        }
    }
    server->window_count = 0;
    server->focused = NULL;
}

struct zenith_window *zenith_window_create(struct zenith_server *server) {
    if (server->window_count >= ZENITH_MAX_WINDOWS) return NULL;
    struct zenith_window *win = calloc(1, sizeof(*win));
    if (!win) return NULL;
    win->server = server;
    win->x = 20 + (server->window_count * 30) % 400;
    win->y = ZENITH_TASKBAR_HEIGHT + 20 + (server->window_count * 20) % 300;
    win->width = 800;
    win->height = 600;
    win->mapped = 0;
    win->minimized = 0;
    win->maximized = 0;
    win->fullscreen = 0;
    strncpy(win->title, "Window", sizeof(win->title)-1);
    strncpy(win->app_id, "unknown", sizeof(win->app_id)-1);
    server->windows[server->window_count++] = win;
    return win;
}

void zenith_window_destroy(struct zenith_window *win) {
    struct zenith_server *server = win->server;
    for (int i = 0; i < server->window_count; i++) {
        if (server->windows[i] == win) {
            server->windows[i] = server->windows[--server->window_count];
            server->windows[server->window_count] = NULL;
            break;
        }
    }
    if (server->focused == win) {
        server->focused = server->window_count > 0 ? server->windows[server->window_count-1] : NULL;
    }
    zenith_taskbar_update(server->taskbar);
    free(win);
}

void zenith_window_focus(struct zenith_server *server, struct zenith_window *win) {
    if (!win) return;
    server->focused = win;
    win->minimized = 0;
    for (int i = 0; i < server->window_count; i++) {
        if (server->windows[i] == win) {
            for (int j = i; j < server->window_count - 1; j++)
                server->windows[j] = server->windows[j+1];
            server->windows[server->window_count-1] = win;
            break;
        }
    }
    zenith_taskbar_update(server->taskbar);
}

void zenith_window_close(struct zenith_window *win) {
    if (!win || !win->xdg_toplevel) return;
    wl_resource_post_event(win->xdg_toplevel,
        XDG_TOPLEVEL_CLOSE_EVENT_SINCE_VERSION >= 1 ? 2 : 0);
}

void zenith_window_minimize(struct zenith_window *win) {
    if (!win) return;
    win->minimized = !win->minimized;
    zenith_taskbar_update(win->server->taskbar);
}

void zenith_window_maximize(struct zenith_window *win) {
    if (!win) return;
    win->maximized = !win->maximized;
    struct zenith_server *server = win->server;
    if (server->output_count > 0) {
        struct zenith_output *out = server->outputs[0];
        if (win->maximized) {
            win->x = 0;
            win->y = ZENITH_TASKBAR_HEIGHT;
            win->width  = out->width;
            win->height = out->height - ZENITH_TASKBAR_HEIGHT;
        }
    }
}

void zenith_window_fullscreen(struct zenith_window *win, int enable) {
    if (!win) return;
    win->fullscreen = enable;
    if (enable) {
        struct zenith_server *server = win->server;
        if (server->output_count > 0) {
            struct zenith_output *out = server->outputs[0];
            win->x = 0; win->y = 0;
            win->width = out->width;
            win->height = out->height;
        }
    }
}

void zenith_window_move(struct zenith_window *win, int x, int y) {
    if (!win) return;
    win->x = x;
    win->y = y;
}

void zenith_window_resize(struct zenith_window *win, int w, int h) {
    if (!win) return;
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win->width  = w;
    win->height = h;
}

void zenith_windows_minimize_all(struct zenith_server *server) {
    int any_visible = 0;
    for (int i = 0; i < server->window_count; i++) {
        if (!server->windows[i]->minimized) { any_visible = 1; break; }
    }
    if (any_visible) {
        for (int i = 0; i < server->window_count; i++)
            server->windows[i]->minimized = 1;
        server->all_minimized = 1;
    } else {
        for (int i = 0; i < server->window_count; i++)
            server->windows[i]->minimized = 0;
        server->all_minimized = 0;
    }
    zenith_taskbar_update(server->taskbar);
}

void zenith_windows_restore_all(struct zenith_server *server) {
    for (int i = 0; i < server->window_count; i++)
        server->windows[i]->minimized = 0;
    server->all_minimized = 0;
    zenith_taskbar_update(server->taskbar);
}
