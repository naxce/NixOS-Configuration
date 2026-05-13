#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <libinput.h>
#include <libudev.h>
#include <wayland-server-core.h>
#include <linux/input-event-codes.h>

#include "zenith.h"
#include "input.h"
#include "output.h"
#include "window.h"
#include "keybinds.h"
#include "taskbar.h"

static int libinput_open_restricted(const char *path, int flags, void *data) {
    (void)data;
    int fd = open(path, flags | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "zenith: failed to open %s: %s\n", path, strerror(errno));
    }
    return fd;
}

static void libinput_close_restricted(int fd, void *data) {
    (void)data;
    close(fd);
}

static const struct libinput_interface libinput_impl = {
    .open_restricted  = libinput_open_restricted,
    .close_restricted = libinput_close_restricted,
};

static int dispatch_libinput(int fd, uint32_t mask, void *data) {
    (void)fd; (void)mask;
    struct zenith_input *input = data;
    libinput_dispatch(input->li);

    struct libinput_event *event;
    while ((event = libinput_get_event(input->li)) != NULL) {
        enum libinput_event_type type = libinput_event_get_type(event);
        struct zenith_server *server = input->server;

        if (type == LIBINPUT_EVENT_POINTER_MOTION) {
            struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);
            double dx = libinput_event_pointer_get_dx(pev);
            double dy = libinput_event_pointer_get_dy(pev);
            input->pointer_x += dx;
            input->pointer_y += dy;

            if (input->pointer_x < 0) input->pointer_x = 0;
            if (input->pointer_y < 0) input->pointer_y = 0;
            if (server->output_count > 0) {
                if (input->pointer_x > server->outputs[0]->width)
                    input->pointer_x = server->outputs[0]->width;
                if (input->pointer_y > server->outputs[0]->height)
                    input->pointer_y = server->outputs[0]->height;
            }

            if (input->drag_active && input->drag_window) {
                int nx = input->drag_win_x + ((int)input->pointer_x - input->drag_start_x);
                int ny = input->drag_win_y + ((int)input->pointer_y - input->drag_start_y);
                if (ny < ZENITH_TASKBAR_HEIGHT) ny = ZENITH_TASKBAR_HEIGHT;
                zenith_window_move(input->drag_window, nx, ny);
            } else if (input->resize_active && input->resize_window) {
                int dw = (int)input->pointer_x - input->resize_start_x;
                int dh = (int)input->pointer_y - input->resize_start_y;
                zenith_window_resize(input->resize_window,
                    input->resize_start_w + dw,
                    input->resize_start_h + dh);
            }
        } else if (type == LIBINPUT_EVENT_POINTER_BUTTON) {
            struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);
            uint32_t btn = libinput_event_pointer_get_button(pev);
            enum libinput_button_state state = libinput_event_pointer_get_button_state(pev);

            int px = (int)input->pointer_x;
            int py = (int)input->pointer_y;

            if (btn == BTN_LEFT) {
                if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
                    struct zenith_window *hit = NULL;
                    for (int i = server->window_count - 1; i >= 0; i--) {
                        struct zenith_window *w = server->windows[i];
                        if (!w->mapped || w->minimized) continue;
                        if (px >= w->x && px < w->x + w->width &&
                            py >= w->y && py < w->y + w->height) {
                            hit = w;
                            break;
                        }
                    }
                    if (hit) {
                        zenith_window_focus(server, hit);
                        if (py < hit->y + 28) {
                            input->drag_active = 1;
                            input->drag_window = hit;
                            input->drag_start_x = px;
                            input->drag_start_y = py;
                            input->drag_win_x = hit->x;
                            input->drag_win_y = hit->y;
                        }
                    }
                } else {
                    input->drag_active = 0;
                    input->drag_window = NULL;
                    input->resize_active = 0;
                    input->resize_window = NULL;
                }
            } else if (btn == BTN_RIGHT && state == LIBINPUT_BUTTON_STATE_PRESSED) {
                struct zenith_window *hit = NULL;
                for (int i = server->window_count - 1; i >= 0; i--) {
                    struct zenith_window *w = server->windows[i];
                    if (!w->mapped || w->minimized) continue;
                    if (px >= w->x && px < w->x + w->width &&
                        py >= w->y - 28 && py < w->y + 28) {
                        hit = w;
                        break;
                    }
                }
                if (hit) {
                    input->resize_active = 1;
                    input->resize_window = hit;
                    input->resize_start_x = px;
                    input->resize_start_y = py;
                    input->resize_start_w = hit->width;
                    input->resize_start_h = hit->height;
                }
            }
        } else if (type == LIBINPUT_EVENT_KEYBOARD_KEY) {
            struct libinput_event_keyboard *kev = libinput_event_get_keyboard_event(event);
            uint32_t key = libinput_event_keyboard_get_key(kev);
            enum libinput_key_state kstate = libinput_event_keyboard_get_key_state(kev);

            if (key == KEY_LEFTSHIFT || key == KEY_RIGHTSHIFT) {
                if (kstate == LIBINPUT_KEY_STATE_PRESSED)
                    input->modifier_state |= ZENITH_MOD_SHIFT;
                else
                    input->modifier_state &= ~ZENITH_MOD_SHIFT;
            } else if (key == KEY_LEFTCTRL || key == KEY_RIGHTCTRL) {
                if (kstate == LIBINPUT_KEY_STATE_PRESSED)
                    input->modifier_state |= ZENITH_MOD_CTRL;
                else
                    input->modifier_state &= ~ZENITH_MOD_CTRL;
            } else if (key == KEY_LEFTALT || key == KEY_RIGHTALT) {
                if (kstate == LIBINPUT_KEY_STATE_PRESSED)
                    input->modifier_state |= ZENITH_MOD_ALT;
                else
                    input->modifier_state &= ~ZENITH_MOD_ALT;
            } else if (key == KEY_LEFTMETA || key == KEY_RIGHTMETA) {
                if (kstate == LIBINPUT_KEY_STATE_PRESSED)
                    input->modifier_state |= ZENITH_MOD_SUPER;
                else
                    input->modifier_state &= ~ZENITH_MOD_SUPER;
            }

            if (kstate == LIBINPUT_KEY_STATE_PRESSED) {
                zenith_keybinds_handle(server->keybinds, key, input->modifier_state);
            }
        }

        libinput_event_destroy(event);
    }
    return 0;
}

int zenith_input_init(struct zenith_server *server) {
    struct zenith_input *input = calloc(1, sizeof(*input));
    if (!input) return 0;
    input->server = server;

    input->udev = udev_new();
    if (!input->udev) {
        fprintf(stderr, "zenith: udev_new failed\n");
        free(input);
        return 0;
    }

    input->li = libinput_udev_create_context(&libinput_impl, input, input->udev);
    if (!input->li) {
        fprintf(stderr, "zenith: libinput_udev_create_context failed\n");
        udev_unref(input->udev);
        free(input);
        return 0;
    }

    const char *seat = getenv("XDG_SEAT");
    if (!seat) seat = "seat0";
    if (libinput_udev_assign_seat(input->li, seat) < 0) {
        fprintf(stderr, "zenith: libinput_udev_assign_seat failed\n");
        libinput_unref(input->li);
        udev_unref(input->udev);
        free(input);
        return 0;
    }

    int fd = libinput_get_fd(input->li);
    input->event_source = wl_event_loop_add_fd(server->event_loop, fd,
        WL_EVENT_READABLE, dispatch_libinput, input);

    if (server->output_count > 0) {
        input->pointer_x = server->outputs[0]->width / 2.0;
        input->pointer_y = server->outputs[0]->height / 2.0;
    }

    server->input = input;
    return 1;
}

void zenith_input_destroy(struct zenith_server *server) {
    if (!server->input) return;
    struct zenith_input *input = server->input;
    if (input->event_source) wl_event_source_remove(input->event_source);
    if (input->li) libinput_unref(input->li);
    if (input->udev) udev_unref(input->udev);
    free(input);
    server->input = NULL;
}
