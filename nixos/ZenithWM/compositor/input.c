/*
 * ZenithWM - input.c
 * Keyboard, pointer, keybinding dispatch.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "input.h"

/* ------------------------------------------------------------------ */
/*  Keybinding config                                                   */
/* ------------------------------------------------------------------ */

#define MAX_BINDINGS 64

typedef enum {
    ACTION_EXEC,
    ACTION_CLOSE_WINDOW,
    ACTION_FULLSCREEN,
    ACTION_MINIMIZE,
    ACTION_SHOW_DESKTOP,
    ACTION_WORKSPACE,
    ACTION_MOVE_TO_WORKSPACE,
    ACTION_EXEC_TERMINAL,
} KeyAction;

struct keybind {
    uint32_t mods;
    xkb_keysym_t sym;
    KeyAction action;
    char arg[256]; /* for EXEC or workspace number */
};

static struct keybind bindings[MAX_BINDINGS];
static int n_bindings = 0;

static uint32_t parse_mods(const char *s) {
    uint32_t mods = 0;
    char buf[256];
    strncpy(buf, s, sizeof(buf) - 1);
    char *tok = strtok(buf, "+");
    while (tok) {
        if (strcasecmp(tok, "Super") == 0 || strcasecmp(tok, "Mod4") == 0)
            mods |= WLR_MODIFIER_LOGO;
        else if (strcasecmp(tok, "Alt") == 0 || strcasecmp(tok, "Mod1") == 0)
            mods |= WLR_MODIFIER_ALT;
        else if (strcasecmp(tok, "Ctrl") == 0)
            mods |= WLR_MODIFIER_CTRL;
        else if (strcasecmp(tok, "Shift") == 0)
            mods |= WLR_MODIFIER_SHIFT;
        tok = strtok(NULL, "+");
    }
    return mods;
}

void zenith_keybinds_load(void) {
    n_bindings = 0;

    /* Built-in defaults */
    struct { const char *combo; KeyAction act; const char *arg; } defaults[] = {
        { "Super+Return",   ACTION_EXEC_TERMINAL, "" },
        { "Super+q",        ACTION_CLOSE_WINDOW,  "" },
        { "Super+f",        ACTION_FULLSCREEN,    "" },
        { "Super+m",        ACTION_MINIMIZE,      "" },
        { "Super+d",        ACTION_SHOW_DESKTOP,  "" },
        { "Super+1",        ACTION_WORKSPACE,     "1" },
        { "Super+2",        ACTION_WORKSPACE,     "2" },
        { "Super+3",        ACTION_WORKSPACE,     "3" },
        { "Super+4",        ACTION_WORKSPACE,     "4" },
        { "Super+5",        ACTION_WORKSPACE,     "5" },
        { "Super+Shift+1",  ACTION_MOVE_TO_WORKSPACE, "1" },
        { "Super+Shift+2",  ACTION_MOVE_TO_WORKSPACE, "2" },
        { "Super+Shift+3",  ACTION_MOVE_TO_WORKSPACE, "3" },
        { NULL, 0, NULL }
    };

    for (int i = 0; defaults[i].combo && n_bindings < MAX_BINDINGS; i++) {
        char buf[256];
        strncpy(buf, defaults[i].combo, sizeof(buf) - 1);
        /* Last token after final '+' is the key */
        char *last = strrchr(buf, '+');
        if (!last) continue;
        *last = '\0';
        uint32_t mods = parse_mods(buf);
        xkb_keysym_t sym = xkb_keysym_from_name(last + 1, XKB_KEYSYM_CASE_INSENSITIVE);
        if (sym == XKB_KEY_NoSymbol) continue;
        bindings[n_bindings].mods = mods;
        bindings[n_bindings].sym  = sym;
        bindings[n_bindings].action = defaults[i].act;
        strncpy(bindings[n_bindings].arg, defaults[i].arg,
                sizeof(bindings[n_bindings].arg) - 1);
        n_bindings++;
    }

    /* Load user overrides from keys.conf */
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/keys.conf", home);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f) && n_bindings < MAX_BINDINGS) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char combo[128], action_str[256];
        if (sscanf(line, " %127[^= ] = %255[^\n]", combo, action_str) != 2) continue;

        /* Trim whitespace */
        char *p = combo + strlen(combo) - 1;
        while (p > combo && (*p == ' ' || *p == '\t')) *p-- = '\0';
        p = action_str;
        while (*p == ' ' || *p == '\t') p++;

        char buf[128];
        strncpy(buf, combo, sizeof(buf) - 1);
        char *last = strrchr(buf, '+');
        if (!last) continue;
        *last = '\0';
        uint32_t mods = parse_mods(buf);
        xkb_keysym_t sym = xkb_keysym_from_name(last + 1, XKB_KEYSYM_CASE_INSENSITIVE);
        if (sym == XKB_KEY_NoSymbol) continue;

        KeyAction act;
        char arg[256] = "";
        if (strncmp(p, "exec ", 5) == 0) {
            act = ACTION_EXEC;
            strncpy(arg, p + 5, sizeof(arg) - 1);
        } else if (strcmp(p, "close_window") == 0) {
            act = ACTION_CLOSE_WINDOW;
        } else if (strcmp(p, "fullscreen") == 0) {
            act = ACTION_FULLSCREEN;
        } else if (strcmp(p, "minimize") == 0) {
            act = ACTION_MINIMIZE;
        } else if (strcmp(p, "show_desktop") == 0) {
            act = ACTION_SHOW_DESKTOP;
        } else if (strncmp(p, "workspace ", 10) == 0) {
            act = ACTION_WORKSPACE;
            strncpy(arg, p + 10, sizeof(arg) - 1);
        } else if (strncmp(p, "move_to_workspace ", 18) == 0) {
            act = ACTION_MOVE_TO_WORKSPACE;
            strncpy(arg, p + 18, sizeof(arg) - 1);
        } else {
            continue;
        }

        bindings[n_bindings].mods = mods;
        bindings[n_bindings].sym  = sym;
        bindings[n_bindings].action = act;
        strncpy(bindings[n_bindings].arg, arg, sizeof(bindings[n_bindings].arg) - 1);
        n_bindings++;
    }
    fclose(f);
}

static void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(1);
    }
}

static bool handle_keybind(struct zenith_server *server, uint32_t mods, xkb_keysym_t sym) {
    for (int i = 0; i < n_bindings; i++) {
        if (bindings[i].mods != mods) continue;
        if (bindings[i].sym  != sym)  continue;

        switch (bindings[i].action) {
        case ACTION_EXEC:
            spawn(bindings[i].arg);
            return true;
        case ACTION_EXEC_TERMINAL:
            spawn(server->config.terminal);
            return true;
        case ACTION_CLOSE_WINDOW:
            if (server->focused_view)
                wlr_xdg_toplevel_send_close(server->focused_view->xdg_toplevel);
            return true;
        case ACTION_FULLSCREEN:
            if (server->focused_view) {
                bool fs = !server->focused_view->fullscreen;
                wlr_xdg_toplevel_set_fullscreen(server->focused_view->xdg_toplevel, fs);
                server->focused_view->fullscreen = fs;
            }
            return true;
        case ACTION_MINIMIZE:
            if (server->focused_view) {
                server->focused_view->minimized = true;
                wlr_scene_node_set_enabled(
                    &server->focused_view->scene_tree->node, false);
            }
            return true;
        case ACTION_SHOW_DESKTOP: {
            /* Toggle: if any visible, hide all; else show all */
            bool any_visible = false;
            struct zenith_view *v;
            wl_list_for_each(v, &server->views, link) {
                if (!v->minimized) { any_visible = true; break; }
            }
            wl_list_for_each(v, &server->views, link) {
                v->minimized = any_visible;
                wlr_scene_node_set_enabled(&v->scene_tree->node, !any_visible);
            }
            return true;
        }
        case ACTION_WORKSPACE:
            zenith_switch_workspace(server, atoi(bindings[i].arg) - 1);
            return true;
        case ACTION_MOVE_TO_WORKSPACE:
            if (server->focused_view) {
                int ws = atoi(bindings[i].arg) - 1;
                struct zenith_view *v = server->focused_view;
                wl_list_remove(&v->workspace_link);
                wl_list_insert(&server->workspaces[ws].views, &v->workspace_link);
                v->workspace = ws;
                if (ws != server->active_workspace)
                    wlr_scene_node_set_enabled(&v->scene_tree->node, false);
            }
            return true;
        }
    }
    return false;
}

/* ------------------------------------------------------------------ */
/*  Keyboard                                                            */
/* ------------------------------------------------------------------ */

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_keyboard *kb = wl_container_of(listener, kb, modifiers);
    wlr_seat_set_keyboard(kb->server->seat, kb->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(kb->server->seat, &kb->wlr_keyboard->modifiers);
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    struct zenith_keyboard *kb = wl_container_of(listener, kb, key);
    struct zenith_server *server = kb->server;
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = server->seat;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(kb->wlr_keyboard->xkb_state, keycode, &syms);

    uint32_t mods = wlr_keyboard_get_modifiers(kb->wlr_keyboard);
    bool handled = false;

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybind(server, mods, syms[i]);
            if (handled) break;
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, kb->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec,
            event->keycode, event->state);
    }
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_keyboard *kb = wl_container_of(listener, kb, destroy);
    wl_list_remove(&kb->modifiers.link);
    wl_list_remove(&kb->key.link);
    wl_list_remove(&kb->destroy.link);
    wl_list_remove(&kb->link);
    free(kb);
}

static void server_new_keyboard(struct zenith_server *server, struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    struct zenith_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
    keyboard->destroy.notify = keyboard_handle_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(server->seat, wlr_keyboard);
    wl_list_insert(&server->keyboards, &keyboard->link);
}

/* ------------------------------------------------------------------ */
/*  Pointer                                                             */
/* ------------------------------------------------------------------ */

static void server_cursor_motion(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(server->cursor, &event->pointer->base,
        event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

static void server_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x, event->y);
    process_cursor_motion(server, event->time_msec);
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;

    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
        double sx, sy;
        struct wlr_surface *surface = NULL;
        struct zenith_view *view = zenith_view_at(server,
            server->cursor->x, server->cursor->y, &surface, &sx, &sy);
        if (view) zenith_focus_view(view, surface);
    }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    wlr_seat_pointer_notify_axis(server->seat, event->time_msec,
        event->orientation, event->delta, event->delta_discrete, event->source,
        event->relative_direction);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_server *server = wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

void process_cursor_motion(struct zenith_server *server, uint32_t time) {
    if (server->grab.mode == ZENITH_CURSOR_MOVE) {
        struct zenith_view *view = server->grab.view;
        view->x = server->cursor->x - server->grab.x;
        view->y = server->cursor->y - server->grab.y;
        wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);
        return;
    }
    if (server->grab.mode == ZENITH_CURSOR_RESIZE) {
        struct zenith_view *view = server->grab.view;
        double bx = server->cursor->x - server->grab.x;
        double by = server->cursor->y - server->grab.y;
        struct wlr_box new_geo = server->grab.geo;
        int new_w = new_geo.width, new_h = new_geo.height;
        if (server->grab.resize_edges & WLR_EDGE_RIGHT)  new_w = bx;
        if (server->grab.resize_edges & WLR_EDGE_BOTTOM) new_h = by;
        if (server->grab.resize_edges & WLR_EDGE_LEFT) {
            new_w = new_geo.width - bx;
            view->x = server->grab.geo.x + (int)bx;
        }
        if (server->grab.resize_edges & WLR_EDGE_TOP) {
            new_h = new_geo.height - by;
            view->y = server->grab.geo.y + (int)by;
        }
        wlr_xdg_toplevel_set_size(view->xdg_toplevel,
            new_w < 1 ? 1 : new_w, new_h < 1 ? 1 : new_h);
        wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);
        return;
    }

    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct zenith_view *view = zenith_view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (!view) {
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
        wlr_seat_pointer_clear_focus(server->seat);
    } else {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    }
}

/* ------------------------------------------------------------------ */
/*  Seat requests                                                       */
/* ------------------------------------------------------------------ */

void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused = server->seat->pointer_state.focused_client;
    if (focused == event->seat_client)
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
}

void seat_request_set_selection(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

/* ------------------------------------------------------------------ */
/*  New input device                                                    */
/* ------------------------------------------------------------------ */

static void server_new_input(struct wl_listener *listener, void *data) {
    struct zenith_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        server_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        wlr_cursor_attach_input_device(server->cursor, device);
        break;
    default:
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards))
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    wlr_seat_set_capabilities(server->seat, caps);
}

void zenith_input_init(struct zenith_server *server) {
    wl_list_init(&server->keyboards);

    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    wlr_xcursor_manager_load(server->cursor_mgr, 1);

    server->cursor_motion.notify = server_cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
    server->cursor_motion_absolute.notify = server_cursor_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute, &server->cursor_motion_absolute);
    server->cursor_button.notify = server_cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);
    server->cursor_axis.notify = server_cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);
    server->cursor_frame.notify = server_cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    server->new_input.notify = server_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);

    zenith_keybinds_load();
}