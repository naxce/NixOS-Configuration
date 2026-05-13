/*
 * ZenithWM - xdg_shell.c
 * XDG toplevel (normal windows) + layer shell (taskbar).
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "xdg_shell.h"

/* ------------------------------------------------------------------ */
/*  XDG Toplevel                                                        */
/* ------------------------------------------------------------------ */

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, map);
    wl_list_insert(&view->server->views, &view->link);
    wl_list_insert(&view->server->workspaces[view->server->active_workspace].views,
                   &view->workspace_link);
    view->workspace = view->server->active_workspace;
    zenith_focus_view(view, view->xdg_toplevel->base->surface);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, unmap);
    if (view == view->server->focused_view)
        view->server->focused_view = NULL;
    wl_list_remove(&view->link);
    wl_list_remove(&view->workspace_link);
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, destroy);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);
    free(view);
}

static void begin_interactive(struct zenith_view *view,
        enum { ZENITH_CURSOR_MOVE, ZENITH_CURSOR_RESIZE } mode, uint32_t edges) {
    struct zenith_server *server = view->server;
    struct wlr_surface *focused = server->seat->pointer_state.focused_surface;
    if (!view->xdg_toplevel->base->surface ||
        view->xdg_toplevel->base->surface != focused) return;

    server->grab.view = view;
    server->grab.mode = mode;

    if (mode == ZENITH_CURSOR_MOVE) {
        server->grab.x = server->cursor->x - view->x;
        server->grab.y = server->cursor->y - view->y;
    } else {
        struct wlr_box geo;
        wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
        server->grab.x = server->cursor->x - view->x;
        server->grab.y = server->cursor->y - view->y;
        server->grab.geo = (struct wlr_box){view->x, view->y, geo.width, geo.height};
        server->grab.resize_edges = edges;
    }
}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, request_move);
    begin_interactive(view, ZENITH_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
    struct zenith_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    begin_interactive(view, ZENITH_CURSOR_RESIZE, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, request_maximize);
    /* For gaming: maximize = fill output */
    struct wlr_output *output = wlr_output_layout_output_at(
        view->server->output_layout,
        view->server->cursor->x, view->server->cursor->y);
    if (output) {
        wlr_xdg_toplevel_set_size(view->xdg_toplevel,
            output->width, output->height);
        view->x = 0;
        /* Reserve space for taskbar (32px) */
        view->y = 32;
        wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);
    }
    wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, request_fullscreen);
    bool fs = view->xdg_toplevel->requested.fullscreen;
    wlr_xdg_toplevel_set_fullscreen(view->xdg_toplevel, fs);
    view->fullscreen = fs;
    if (fs) {
        struct wlr_output *output = wlr_output_layout_output_at(
            view->server->output_layout,
            view->server->cursor->x, view->server->cursor->y);
        if (output) {
            wlr_xdg_toplevel_set_size(view->xdg_toplevel, output->width, output->height);
            view->x = 0; view->y = 0;
            wlr_scene_node_set_position(&view->scene_tree->node, 0, 0);
        }
    }
    wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

void server_new_xdg_surface(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, new_xdg_surface);
    struct wlr_xdg_toplevel *toplevel = data;

    struct zenith_view *view = calloc(1, sizeof(*view));
    view->server = server;
    view->xdg_toplevel = toplevel;
    view->scene_tree = wlr_scene_xdg_surface_create(
        &server->scene->tree, toplevel->base);
    view->scene_tree->node.data = view;
    toplevel->base->data = view->scene_tree;

    view->map.notify = xdg_toplevel_map;
    wl_signal_add(&toplevel->base->surface->events.map, &view->map);
    view->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&toplevel->base->surface->events.unmap, &view->unmap);
    view->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&toplevel->events.destroy, &view->destroy);
    view->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    view->request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
    view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);
}

/* ------------------------------------------------------------------ */
/*  Layer shell (taskbar lives here)                                    */
/* ------------------------------------------------------------------ */

void server_new_layer_surface(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, new_layer_surface);
    struct wlr_layer_surface_v1 *layer_surface = data;

    /* Place layer surfaces on the scene above regular windows */
    struct wlr_scene_layer_surface_v1 *scene_layer =
        wlr_scene_layer_surface_v1_create(&server->scene->tree, layer_surface);
    (void)scene_layer;

    /* Auto-configure if the client didn't set a size */
    if (!layer_surface->current.desired_width) {
        struct wlr_output *output = layer_surface->output;
        if (!output) {
            output = wlr_output_layout_output_at(server->output_layout, 0, 0);
            layer_surface->output = output;
        }
        if (output) {
            wlr_layer_surface_v1_configure(layer_surface, output->width, 32);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  XWayland                                                            */
/* ------------------------------------------------------------------ */

#ifdef WLR_HAS_XWAYLAND
void server_new_xwayland_surface(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, new_xwayland_surface);
    struct wlr_xwayland_surface *xsurface = data;
    /* Basic XWayland support: map it as a scene surface */
    (void)server;
    (void)xsurface;
    /* Full XWayland view handling would mirror the XDG path above;
       abbreviated here for clarity — extend as needed for legacy games. */
}
#endif