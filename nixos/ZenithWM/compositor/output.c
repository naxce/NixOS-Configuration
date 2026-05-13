/*
 * ZenithWM - output.c
 * Apply saved monitor configuration when an output is connected.
 * Config format matches what zenith-monitors writes.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "output.h"

/*
 * monitors.conf format (written by zenith-monitors):
 *
 * [output]
 * name = HDMI-A-1
 * width = 1920
 * height = 1080
 * refresh = 144000   # mHz
 * x = 0
 * y = 0
 * scale = 1.0
 * transform = normal
 * adaptive_sync = true
 * enabled = true
 */

struct saved_output {
    char name[64];
    int width, height;
    int refresh; /* mHz */
    int x, y;
    float scale;
    enum wl_output_transform transform;
    bool adaptive_sync;
    bool enabled;
};

static bool load_output_config(const char *output_name, struct saved_output *out) {
    const char *home = getenv("HOME");
    if (!home) return false;

    char path[512];
    snprintf(path, sizeof(path), "%s/.config/zenithwm/monitors.conf", home);
    FILE *f = fopen(path, "r");
    if (!f) return false;

    bool in_section = false;
    bool found = false;
    struct saved_output cur = {
        .width = 0, .height = 0, .refresh = 60000,
        .x = 0, .y = 0, .scale = 1.0f,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .adaptive_sync = false, .enabled = true
    };

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* Trim newline */
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "[output]") == 0) {
            if (in_section && cur.name[0] && strcmp(cur.name, output_name) == 0) {
                *out = cur;
                found = true;
                break;
            }
            in_section = true;
            memset(&cur, 0, sizeof(cur));
            cur.scale = 1.0f;
            cur.refresh = 60000;
            cur.enabled = true;
            continue;
        }

        if (!in_section) continue;
        if (line[0] == '#' || line[0] == '\0') continue;

        char key[64], val[192];
        if (sscanf(line, " %63[^= ] = %191[^\n]", key, val) != 2) continue;

        if (strcmp(key, "name") == 0)
            strncpy(cur.name, val, sizeof(cur.name) - 1);
        else if (strcmp(key, "width") == 0)  cur.width   = atoi(val);
        else if (strcmp(key, "height") == 0) cur.height  = atoi(val);
        else if (strcmp(key, "refresh") == 0) cur.refresh = atoi(val);
        else if (strcmp(key, "x") == 0)      cur.x       = atoi(val);
        else if (strcmp(key, "y") == 0)      cur.y       = atoi(val);
        else if (strcmp(key, "scale") == 0)  cur.scale   = atof(val);
        else if (strcmp(key, "enabled") == 0) cur.enabled = (strcmp(val,"true")==0);
        else if (strcmp(key, "adaptive_sync") == 0) cur.adaptive_sync = (strcmp(val,"true")==0);
        else if (strcmp(key, "transform") == 0) {
            if (strcmp(val, "90") == 0)          cur.transform = WL_OUTPUT_TRANSFORM_90;
            else if (strcmp(val, "180") == 0)    cur.transform = WL_OUTPUT_TRANSFORM_180;
            else if (strcmp(val, "270") == 0)    cur.transform = WL_OUTPUT_TRANSFORM_270;
            else if (strcmp(val, "flipped") == 0) cur.transform = WL_OUTPUT_TRANSFORM_FLIPPED;
            else                                  cur.transform = WL_OUTPUT_TRANSFORM_NORMAL;
        }
    }

    /* Last section */
    if (in_section && !found && cur.name[0] && strcmp(cur.name, output_name) == 0) {
        *out = cur;
        found = true;
    }

    fclose(f);
    return found;
}

void zenith_output_apply_config(struct zenith_server *server, struct wlr_output *wlr_output) {
    struct saved_output saved;
    bool have_saved = load_output_config(wlr_output->name, &saved);

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    if (have_saved && !saved.enabled) {
        wlr_output_state_set_enabled(&state, false);
        wlr_output_commit_state(wlr_output, &state);
        wlr_output_state_finish(&state);
        return;
    }

    wlr_output_state_set_enabled(&state, true);

    /* Find matching mode */
    struct wlr_output_mode *mode = NULL;
    if (have_saved && saved.width > 0 && saved.height > 0) {
        struct wlr_output_mode *m;
        wl_list_for_each(m, &wlr_output->modes, link) {
            if (m->width == saved.width && m->height == saved.height) {
                if (!mode || abs(m->refresh - saved.refresh) <
                             abs(mode->refresh - saved.refresh))
                    mode = m;
            }
        }
    }

    if (!mode && !wl_list_empty(&wlr_output->modes)) {
        /* Preferred mode */
        mode = wl_container_of(wlr_output->modes.prev, mode, link);
    }

    if (mode)
        wlr_output_state_set_mode(&state, mode);

    if (have_saved) {
        wlr_output_state_set_scale(&state, saved.scale);
        wlr_output_state_set_transform(&state, saved.transform);
        if (saved.adaptive_sync)
            wlr_output_state_set_adaptive_sync_enabled(&state, true);
    } else {
        wlr_output_state_set_scale(&state, 1.0f);
        if (server->config.adaptive_sync)
            wlr_output_state_set_adaptive_sync_enabled(&state, true);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);
}