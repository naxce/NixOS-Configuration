/*
 * ZenithWM - server.c
 * Core compositor: output management, scene graph, XDG shell, layer shell.
 */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "server.h"
#include "output.h"
#include "input.h"
#include "xdg_shell.h"

/* ------------------------------------------------------------------ */
/*  Config                                                              */
/* ------------------------------------------------------------------ */

void zenith_config_load(struct zenith_config *cfg) {
    /* Defaults */
    cfg->adaptive_sync  = true;
    cfg->vsync          = false;
    cfg->xwayland       = true;
    cfg->border_width   = 1;
    cfg->border_color   = 0x333333FF;
    cfg->no_animations  = true;
    strncpy(cfg->terminal, "foot", sizeof(cfg->terminal) - 1);

    char path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.config/zenithwm/zenithwm.conf", home);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '[' || line[0] == '\n') continue;
        char key[64], val[192];
        if (sscanf(line, " %63[^= ] = %191[^\n]", key, val) != 2) continue;

        if (strcmp(key, "adaptive_sync") == 0)
            cfg->adaptive_sync = (strcmp(val, "true") == 0);
        else if (strcmp(key, "vsync") == 0)
            cfg->vsync = (strcmp(val, "true") == 0);
        else if (strcmp(key, "xwayland") == 0)
            cfg->xwayland = (strcmp(val, "true") == 0);
        else if (strcmp(key, "border_width") == 0)
            cfg->border_width = atoi(val);
        else if (strcmp(key, "no_animations") == 0)
            cfg->no_animations = (strcmp(val, "true") == 0);
        else if (strcmp(key, "terminal") == 0)
            strncpy(cfg->terminal, val, sizeof(cfg->terminal) - 1);
    }
    fclose(f);
}

/* Ensure ~/.config/zenithwm/ exists and has default configs */
static void ensure_config_dir(void) {
    const char *home = getenv("HOME");
    if (!home) return;

    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/zenithwm", home);
    mkdir(dir, 0755);

    /* Copy defaults if not present */
    const char *defaults[] = {
        "zenithwm.conf", "keys.conf", "desktop.conf", "monitors.conf", NULL
    };
    for (int i = 0; defaults[i]; i++) {
        char dst[512], src[512];
        snprintf(dst, sizeof(dst), "%s/%s", dir, defaults[i]);
        snprintf(src, sizeof(src), ZENITH_DATA_DIR "/defaults/%s", defaults[i]);
        struct stat st;
        if (stat(dst, &st) != 0) {
            /* File doesn't exist, copy from defaults */
            FILE *in = fopen(src, "r");
            FILE *out = fopen(dst, "w");
            if (in && out) {
                char buf[4096];
                size_t n;
                while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
                    fwrite(buf, 1, n, out);
            }
            if (in)  fclose(in);
            if (out) fclose(out);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Output                                                              */
/* ------------------------------------------------------------------ */

static void output_frame(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;
    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);

    wlr_scene_output_commit(scene_output, NULL);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
    struct zenith_output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_commit_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    /* Apply saved monitor config */
    zenith_output_apply_config(server, wlr_output);

    struct zenith_output *output = calloc(1, sizeof(*output));
    output->wlr_output = wlr_output;
    output->server = server;
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->request_state.notify = output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);
    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    wl_list_insert(&server->outputs, &output->link);

    struct wlr_output_layout_output *l_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);
    output->scene_output = wlr_scene_output_create(server->scene, wlr_output);
    wlr_scene_output_layout_add_output(server->scene_layout, l_output, output->scene_output);
}

/* ------------------------------------------------------------------ */
/*  Init / Run / Destroy                                                */
/* ------------------------------------------------------------------ */

bool zenith_server_init(struct zenith_server *server) {
    ensure_config_dir();
    zenith_config_load(&server->config);

    server->wl_display = wl_display_create();
    if (!server->wl_display) return false;

    server->backend = wlr_backend_autocreate(
        wl_display_get_event_loop(server->wl_display), NULL);
    if (!server->backend) {
        wl_display_destroy(server->wl_display);
        return false;
    }

    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) return false;
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);

    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
    if (!server->allocator) return false;

    /* Core protocols */
    wlr_compositor_create(server->wl_display, 5, server->renderer);
    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);

    /* Scene graph */
    server->scene = wlr_scene_create();
    server->output_layout = wlr_output_layout_create(server->wl_display);
    server->scene_layout = wlr_scene_attach_output_layout(server->scene, server->output_layout);

    /* Outputs */
    wl_list_init(&server->outputs);
    server->new_output.notify = server_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    /* XDG shell */
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    wl_list_init(&server->views);
    server->new_xdg_surface.notify = server_new_xdg_surface;
    wl_signal_add(&server->xdg_shell->events.new_toplevel, &server->new_xdg_surface);

    /* Layer shell (for taskbar) */
    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display, 4);
    server->new_layer_surface.notify = server_new_layer_surface;
    wl_signal_add(&server->layer_shell->events.new_surface, &server->new_layer_surface);

    /* Extra protocols */
    server->xdg_output_manager = wlr_xdg_output_manager_v1_create(
        server->wl_display, server->output_layout);
    server->gamma_control_manager = wlr_gamma_control_manager_v1_create(server->wl_display);
    server->screencopy_manager = wlr_screencopy_manager_v1_create(server->wl_display);
    server->viewporter = wlr_viewporter_create(server->wl_display);
    server->presentation = wlr_presentation_create(server->wl_display, server->backend);

    /* Workspaces */
    for (int i = 0; i < MAX_WORKSPACES; i++) {
        server->workspaces[i].index = i;
        wl_list_init(&server->workspaces[i].views);
    }
    server->active_workspace = 0;

    /* Input */
    zenith_input_init(server);

    /* Seat */
    server->seat = wlr_seat_create(server->wl_display, "seat0");
    server->request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor, &server->request_cursor);
    server->request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection, &server->request_set_selection);

#ifdef WLR_HAS_XWAYLAND
    if (server->config.xwayland) {
        server->xwayland = wlr_xwayland_create(server->wl_display, NULL, true);
        if (server->xwayland) {
            server->new_xwayland_surface.notify = server_new_xwayland_surface;
            wl_signal_add(&server->xwayland->events.new_surface,
                          &server->new_xwayland_surface);
            setenv("DISPLAY", server->xwayland->display_name, true);
            wlr_log(WLR_INFO, "XWayland on %s", server->xwayland->display_name);
        }
    }
#endif

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return false;
    }

    return true;
}

void zenith_server_run(struct zenith_server *server) {
    wl_display_run(server->wl_display);
}

void zenith_server_destroy(struct zenith_server *server) {
    wl_display_destroy_clients(server->wl_display);
#ifdef WLR_HAS_XWAYLAND
    if (server->xwayland)
        wlr_xwayland_destroy(server->xwayland);
#endif
    wlr_scene_node_destroy(&server->scene->tree.node);
    wlr_output_layout_destroy(server->output_layout);
    wlr_allocator_destroy(server->allocator);
    wlr_renderer_destroy(server->renderer);
    wlr_backend_destroy(server->backend);
    wl_display_destroy(server->wl_display);
}

/* ------------------------------------------------------------------ */
/*  Workspace switching                                                 */
/* ------------------------------------------------------------------ */

void zenith_switch_workspace(struct zenith_server *server, int ws) {
    if (ws < 0 || ws >= MAX_WORKSPACES) return;
    if (ws == server->active_workspace) return;

    /* Hide current workspace views */
    struct zenith_view *view;
    wl_list_for_each(view, &server->workspaces[server->active_workspace].views, workspace_link) {
        wlr_scene_node_set_enabled(&view->scene_tree->node, false);
    }

    server->active_workspace = ws;

    /* Show new workspace views */
    wl_list_for_each(view, &server->workspaces[ws].views, workspace_link) {
        if (!view->minimized)
            wlr_scene_node_set_enabled(&view->scene_tree->node, true);
    }

    /* Focus the top view on new workspace */
    if (!wl_list_empty(&server->workspaces[ws].views)) {
        view = wl_container_of(server->workspaces[ws].views.next, view, workspace_link);
        zenith_focus_view(view, view->xdg_toplevel->base->surface);
    }
}

void zenith_focus_view(struct zenith_view *view, struct wlr_surface *surface) {
    if (!view) return;
    struct zenith_server *server = view->server;
    struct wlr_surface *prev_surface = server->seat->keyboard_state.focused_surface;

    if (prev_surface == surface) return;

    if (prev_surface) {
        struct wlr_xdg_toplevel *prev =
            wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        if (prev)
            wlr_xdg_toplevel_set_activated(prev, false);
    }

    wlr_scene_node_raise_to_top(&view->scene_tree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);

    wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);
    server->focused_view = view;

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);
    if (keyboard)
        wlr_seat_keyboard_notify_enter(server->seat, surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

struct zenith_view *zenith_view_at(struct zenith_server *server, double lx, double ly,
        struct wlr_surface **surface, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
    if (!node || node->type != WLR_SCENE_NODE_BUFFER) return NULL;

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) return NULL;

    *surface = scene_surface->surface;
    struct wlr_scene_tree *tree = node->parent;
    while (tree && !tree->node.data) tree = tree->node.parent;
    return tree ? tree->node.data : NULL;
}