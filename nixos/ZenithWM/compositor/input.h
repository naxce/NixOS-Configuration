#ifndef ZENITH_INPUT_H
#define ZENITH_INPUT_H

#include "server.h"

void zenith_input_init(struct zenith_server *server);
void zenith_keybinds_load(void);
void process_cursor_motion(struct zenith_server *server, uint32_t time);
void seat_request_cursor(struct wl_listener *listener, void *data);
void seat_request_set_selection(struct wl_listener *listener, void *data);

#endif