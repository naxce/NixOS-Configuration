#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "config.h"
#include "zenith.h"

static void ensure_config_dir(struct zenith_config *cfg) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s", cfg->config_dir);
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
    snprintf(path, sizeof(path), "%s/zenith", cfg->config_dir);
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

void zenith_config_init(struct zenith_config *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg) {
        snprintf(cfg->config_dir, sizeof(cfg->config_dir), "%s", xdg);
    } else {
        snprintf(cfg->config_dir, sizeof(cfg->config_dir), "%s/.config", home);
    }
    snprintf(cfg->config_file, sizeof(cfg->config_file), "%s/zenith/zenith.conf", cfg->config_dir);

    strncpy(cfg->terminal, "foot", sizeof(cfg->terminal)-1);
    strncpy(cfg->wallpaper, "", sizeof(cfg->wallpaper)-1);
    cfg->border_width = 1;
    cfg->border_color_focused = 0xFF4488FF;
    cfg->border_color_normal  = 0xFF222222;
    cfg->taskbar_bg = 0xFF111111;
    cfg->taskbar_fg = 0xFFEEEEEE;
    cfg->gaps = 0;
    cfg->nvidia_mode = 1;
}

int zenith_config_load(struct zenith_config *cfg) {
    ensure_config_dir(cfg);
    FILE *f = fopen(cfg->config_file, "r");
    if (!f) {
        zenith_config_save(cfg);
        return 0;
    }
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        char key[128], val[896];
        if (sscanf(line, "%127[^=]=%895s", key, val) != 2) continue;

        if (strcmp(key, "terminal") == 0)
            strncpy(cfg->terminal, val, sizeof(cfg->terminal)-1);
        else if (strcmp(key, "wallpaper") == 0)
            strncpy(cfg->wallpaper, val, sizeof(cfg->wallpaper)-1);
        else if (strcmp(key, "border_width") == 0)
            cfg->border_width = atoi(val);
        else if (strcmp(key, "border_color_focused") == 0)
            cfg->border_color_focused = (unsigned int)strtoul(val, NULL, 16);
        else if (strcmp(key, "border_color_normal") == 0)
            cfg->border_color_normal = (unsigned int)strtoul(val, NULL, 16);
        else if (strcmp(key, "taskbar_bg") == 0)
            cfg->taskbar_bg = (unsigned int)strtoul(val, NULL, 16);
        else if (strcmp(key, "taskbar_fg") == 0)
            cfg->taskbar_fg = (unsigned int)strtoul(val, NULL, 16);
        else if (strcmp(key, "gaps") == 0)
            cfg->gaps = atoi(val);
        else if (strcmp(key, "nvidia_mode") == 0)
            cfg->nvidia_mode = atoi(val);
    }
    fclose(f);
    return 1;
}

int zenith_config_save(struct zenith_config *cfg) {
    ensure_config_dir(cfg);
    FILE *f = fopen(cfg->config_file, "w");
    if (!f) return 0;
    fprintf(f, "terminal=%s\n", cfg->terminal);
    fprintf(f, "wallpaper=%s\n", cfg->wallpaper);
    fprintf(f, "border_width=%d\n", cfg->border_width);
    fprintf(f, "border_color_focused=%08X\n", cfg->border_color_focused);
    fprintf(f, "border_color_normal=%08X\n", cfg->border_color_normal);
    fprintf(f, "taskbar_bg=%08X\n", cfg->taskbar_bg);
    fprintf(f, "taskbar_fg=%08X\n", cfg->taskbar_fg);
    fprintf(f, "gaps=%d\n", cfg->gaps);
    fprintf(f, "nvidia_mode=%d\n", cfg->nvidia_mode);
    for (int i = 0; i < cfg->monitor_count; i++) {
        struct zenith_monitor_config *m = &cfg->monitors[i];
        fprintf(f, "monitor_%d=%s,%d,%d,%dx%d@%d,%.2f,%d,%d\n",
            i, m->name, m->x, m->y, m->width, m->height,
            m->refresh, m->scale, m->transform, m->enabled);
    }
    fclose(f);
    return 1;
}
