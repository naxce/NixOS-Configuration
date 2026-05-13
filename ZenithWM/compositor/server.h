#ifndef ZENITH_SERVER_H
#define ZENITH_SERVER_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/util/log.h>

#ifdef WLR_HAS_XWAYLAND
#include <wlr/xwayland.h>
#endif

#include <xkbcommon/xkbcommon.h>
#include <stdbool.h>

#define MAX_WORKSPACES 9

struct zenith_config {
    bool adaptive_sync;
    bool vsync;
    bool xwayland;
    int border_width;
    uint32_t border_color;
    bool no_animations;
    char terminal[256];
};

struct zenith_workspace {
    int index;
    struct wl_list views; // zenith_view.workspace_link
};

struct zenith_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;

    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_listener new_layer_surface;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    struct wlr_xdg_output_manager_v1 *xdg_output_manager;
    struct wlr_gamma_control_manager_v1 *gamma_control_manager;
    struct wlr_screencopy_manager_v1 *screencopy_manager;
    struct wlr_viewporter *viewporter;
    struct wlr_presentation *presentation;

    struct wl_list views; // all views, active workspace
    struct zenith_workspace workspaces[MAX_WORKSPACES];
    int active_workspace;

    struct zenith_view *focused_view;
    struct zenith_config config;

    /* grab state for move/resize */
    struct {
        double x, y;
        struct wlr_box geo;
        enum { ZENITH_CURSOR_NORMAL, ZENITH_CURSOR_MOVE, ZENITH_CURSOR_RESIZE } mode;
        struct zenith_view *view;
        uint32_t resize_edges;
    } grab;

#ifdef WLR_HAS_XWAYLAND
    struct wlr_xwayland *xwayland;
    struct wl_listener new_xwayland_surface;
#endif
};

struct zenith_output {
    struct wl_list link;
    struct zenith_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

struct zenith_view {
    struct wl_list link;          // server->views
    struct wl_list workspace_link;
    struct zenith_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    int x, y;
    bool minimized;
    bool fullscreen;
    int workspace;
};

struct zenith_keyboard {
    struct wl_list link;
    struct zenith_server *server;
    struct wlr_keyboard *wlr_keyboard;
    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

bool zenith_server_init(struct zenith_server *server);
void zenith_server_run(struct zenith_server *server);
void zenith_server_destroy(struct zenith_server *server);

void zenith_focus_view(struct zenith_view *view, struct wlr_surface *surface);
void zenith_switch_workspace(struct zenith_server *server, int ws);
struct zenith_view *zenith_view_at(struct zenith_server *server, double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy);

void zenith_config_load(struct zenith_config *cfg);

#endif /* ZENITH_SERVER_H */