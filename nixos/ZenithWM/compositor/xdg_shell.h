#ifndef ZENITH_XDG_SHELL_H
#define ZENITH_XDG_SHELL_H

#include "server.h"

void server_new_xdg_surface(struct wl_listener *listener, void *data);
void server_new_layer_surface(struct wl_listener *listener, void *data);

#ifdef WLR_HAS_XWAYLAND
void server_new_xwayland_surface(struct wl_listener *listener, void *data);
#endif

#endif