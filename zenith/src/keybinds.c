#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/input-event-codes.h>

#include "zenith.h"
#include "keybinds.h"
#include "window.h"
#include "input.h"
#include "launcher.h"
#include "config.h"

static void kb_close_focused(struct zenith_server *server) {
    if (server->focused) zenith_window_close(server->focused);
}

static void kb_open_terminal(struct zenith_server *server) {
    const char *term = server->config.terminal;
    if (!term || term[0] == '\0') term = "kitty";
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execlp(term, term, NULL);
        execlp("kitty", "kitty", NULL);
        execlp("alacritty", "alacritty", NULL);
        execlp("xterm", "xterm", NULL);
        _exit(1);
    }
}

static void kb_toggle_launcher(struct zenith_server *server) {
    zenith_launcher_toggle(server->launcher);
}

static void kb_minimize_focused(struct zenith_server *server) {
    if (server->focused) zenith_window_minimize(server->focused);
}

static void kb_maximize_focused(struct zenith_server *server) {
    if (server->focused) zenith_window_maximize(server->focused);
}

static void kb_minimize_all(struct zenith_server *server) {
    zenith_windows_minimize_all(server);
}

static void kb_focus_next(struct zenith_server *server) {
    if (server->window_count < 2) return;
    int idx = -1;
    for (int i = 0; i < server->window_count; i++) {
        if (server->windows[i] == server->focused) { idx = i; break; }
    }
    int next = (idx + 1) % server->window_count;
    zenith_window_focus(server, server->windows[next]);
}

static void kb_quit(struct zenith_server *server) {
    wl_display_terminate(server->display);
}

static void kb_fullscreen_focused(struct zenith_server *server) {
    if (server->focused)
        zenith_window_fullscreen(server->focused, !server->focused->fullscreen);
}

static void kb_reload_config(struct zenith_server *server) {
    zenith_config_load(&server->config);
}

int zenith_keybinds_init(struct zenith_server *server) {
    struct zenith_keybinds *kb = calloc(1, sizeof(*kb));
    if (!kb) return 0;
    kb->server = server;
    kb->count = 0;

    zenith_keybinds_add(kb, KEY_Q,      ZENITH_MOD_SUPER | ZENITH_MOD_SHIFT, kb_close_focused,   "Close focused window");
    zenith_keybinds_add(kb, KEY_ENTER,  ZENITH_MOD_SUPER,                    kb_open_terminal,   "Open terminal");
    zenith_keybinds_add(kb, KEY_SPACE,  ZENITH_MOD_SUPER,                    kb_toggle_launcher, "Toggle launcher");
    zenith_keybinds_add(kb, KEY_H,      ZENITH_MOD_SUPER,                    kb_minimize_focused,"Minimize focused");
    zenith_keybinds_add(kb, KEY_M,      ZENITH_MOD_SUPER,                    kb_maximize_focused,"Maximize focused");
    zenith_keybinds_add(kb, KEY_D,      ZENITH_MOD_SUPER,                    kb_minimize_all,    "Minimize all");
    zenith_keybinds_add(kb, KEY_TAB,    ZENITH_MOD_ALT,                      kb_focus_next,      "Focus next window");
    zenith_keybinds_add(kb, KEY_F11,    0,                                   kb_fullscreen_focused,"Toggle fullscreen");
    zenith_keybinds_add(kb, KEY_R,      ZENITH_MOD_SUPER | ZENITH_MOD_SHIFT, kb_reload_config,   "Reload config");
    zenith_keybinds_add(kb, KEY_F4,     ZENITH_MOD_ALT,                      kb_close_focused,   "Close window (Alt+F4)");
    zenith_keybinds_add(kb, KEY_Q,      ZENITH_MOD_CTRL | ZENITH_MOD_SUPER,  kb_quit,            "Quit Zenith");

    server->keybinds = kb;
    return 1;
}

void zenith_keybinds_destroy(struct zenith_server *server) {
    if (!server->keybinds) return;
    free(server->keybinds);
    server->keybinds = NULL;
}

void zenith_keybinds_handle(struct zenith_keybinds *kb, uint32_t key, uint32_t mods) {
    for (int i = 0; i < kb->count; i++) {
        if (kb->binds[i].key == key && kb->binds[i].modifiers == mods) {
            if (kb->binds[i].handler) kb->binds[i].handler(kb->server);
            return;
        }
    }
}

void zenith_keybinds_add(struct zenith_keybinds *kb, uint32_t key, uint32_t mods,
                          zenith_keybind_fn fn, const char *desc) {
    if (kb->count >= ZENITH_MAX_KEYBINDS) return;
    kb->binds[kb->count].key = key;
    kb->binds[kb->count].modifiers = mods;
    kb->binds[kb->count].handler = fn;
    strncpy(kb->binds[kb->count].description, desc, 63);
    kb->count++;
}
