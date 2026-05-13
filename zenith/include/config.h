#ifndef ZENITH_CONFIG_H
#define ZENITH_CONFIG_H

#include <limits.h>

#define ZENITH_CONFIG_VERSION 1
#define ZENITH_MAX_MONITORS 8

struct zenith_monitor_config {
    char name[64];
    int x, y;
    int width, height;
    int refresh;
    int enabled;
    float scale;
    int transform;
};

struct zenith_config {
    char config_dir[PATH_MAX];
    char config_file[PATH_MAX];

    char terminal[256];
    char wallpaper[PATH_MAX];

    int border_width;
    unsigned int border_color_focused;
    unsigned int border_color_normal;

    unsigned int taskbar_bg;
    unsigned int taskbar_fg;

    int gaps;

    struct zenith_monitor_config monitors[ZENITH_MAX_MONITORS];
    int monitor_count;

    int nvidia_mode;
};

struct zenith_server;

void zenith_config_init(struct zenith_config *cfg);
int  zenith_config_load(struct zenith_config *cfg);
int  zenith_config_save(struct zenith_config *cfg);

#endif
