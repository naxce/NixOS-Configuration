#ifndef ZENITH_INPUT_H
#define ZENITH_INPUT_H

#include <libinput.h>
#include <libudev.h>

struct zenith_server;

struct zenith_input {
    struct zenith_server *server;
    struct libinput *li;
    struct udev *udev;
    struct wl_event_source *event_source;

    double pointer_x;
    double pointer_y;

    int drag_active;
    int drag_start_x;
    int drag_start_y;
    int drag_win_x;
    int drag_win_y;
    struct zenith_window *drag_window;

    int resize_active;
    int resize_start_x;
    int resize_start_y;
    int resize_start_w;
    int resize_start_h;
    struct zenith_window *resize_window;

    uint32_t modifier_state;
};

int  zenith_input_init(struct zenith_server *server);
void zenith_input_destroy(struct zenith_server *server);
void zenith_input_handle_events(struct zenith_input *input);

#define ZENITH_MOD_SHIFT  (1 << 0)
#define ZENITH_MOD_CTRL   (1 << 1)
#define ZENITH_MOD_ALT    (1 << 2)
#define ZENITH_MOD_SUPER  (1 << 3)

#endif
