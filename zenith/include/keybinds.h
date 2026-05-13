#ifndef ZENITH_KEYBINDS_H
#define ZENITH_KEYBINDS_H

#include <stdint.h>
#include <linux/input-event-codes.h>

#define ZENITH_MAX_KEYBINDS 64

struct zenith_server;

typedef void (*zenith_keybind_fn)(struct zenith_server *server);

struct zenith_keybind {
    uint32_t key;
    uint32_t modifiers;
    zenith_keybind_fn handler;
    char description[64];
};

struct zenith_keybinds {
    struct zenith_server *server;
    struct zenith_keybind binds[ZENITH_MAX_KEYBINDS];
    int count;
};

int  zenith_keybinds_init(struct zenith_server *server);
void zenith_keybinds_destroy(struct zenith_server *server);
void zenith_keybinds_handle(struct zenith_keybinds *kb, uint32_t key, uint32_t mods);
void zenith_keybinds_add(struct zenith_keybinds *kb, uint32_t key, uint32_t mods,
                          zenith_keybind_fn fn, const char *desc);

#endif
