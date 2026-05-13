#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include "zenith.h"
#include "monitor_settings.h"
#include "output.h"
#include "config.h"

void zenith_monitor_settings_apply(struct zenith_server *server) {
    for (int i = 0; i < server->output_count; i++) {
        struct zenith_output *out = server->outputs[i];
        if (!out->enabled) continue;
        drmModeSetCrtc(out->drm_fd, out->crtc_id,
            out->front_fb, 0, 0,
            &out->connector_id, 1, &out->mode);
    }

    for (int i = 0; i < server->config.monitor_count; i++) {
        struct zenith_monitor_config *mc = &server->config.monitors[i];
        for (int j = 0; j < server->output_count; j++) {
            struct zenith_output *out = server->outputs[j];
            if (strcmp(out->name, mc->name) == 0) {
                out->x = mc->x;
                out->y = mc->y;
                out->scale = mc->scale;
                out->enabled = mc->enabled;
                break;
            }
        }
    }
}

void zenith_monitor_settings_save(struct zenith_server *server) {
    server->config.monitor_count = server->output_count;
    for (int i = 0; i < server->output_count; i++) {
        struct zenith_output *out = server->outputs[i];
        struct zenith_monitor_config *mc = &server->config.monitors[i];
        strncpy(mc->name, out->name, sizeof(mc->name)-1);
        mc->x = out->x;
        mc->y = out->y;
        mc->width  = out->width;
        mc->height = out->height;
        mc->refresh = out->refresh;
        mc->scale = out->scale;
        mc->enabled = out->enabled;
        mc->transform = 0;
    }
    zenith_config_save(&server->config);
}

void zenith_monitor_settings_load(struct zenith_server *server) {
    for (int i = 0; i < server->config.monitor_count; i++) {
        struct zenith_monitor_config *mc = &server->config.monitors[i];
        for (int j = 0; j < server->output_count; j++) {
            struct zenith_output *out = server->outputs[j];
            if (strcmp(out->name, mc->name) == 0) {
                out->x = mc->x;
                out->y = mc->y;
                out->scale = mc->scale;
                out->enabled = mc->enabled;
                break;
            }
        }
    }
}
