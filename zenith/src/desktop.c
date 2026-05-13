#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#include <cairo/cairo.h>

#include "zenith.h"
#include "desktop.h"
#include "output.h"
#include "config.h"

static void load_desktop_icons(struct zenith_desktop *desk) {
    desk->icon_count = 0;
    const char *home = getenv("HOME");
    if (!home) return;

    char desktop_path[PATH_MAX];
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", home);

    struct stat st;
    if (stat(desktop_path, &st) < 0) {
        mkdir(desktop_path, 0755);
        return;
    }

    DIR *d = opendir(desktop_path);
    if (!d) return;

    struct dirent *ent;
    int col = 0, row = 0;
    while ((ent = readdir(d)) != NULL && desk->icon_count < ZENITH_MAX_ICONS) {
        if (ent->d_name[0] == '.') continue;
        struct zenith_desktop_icon *ic = &desk->icons[desk->icon_count];
        strncpy(ic->label, ent->d_name, 63);

        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", desktop_path, ent->d_name);

        if (strstr(ent->d_name, ".desktop")) {
            FILE *f = fopen(full, "r");
            if (f) {
                char line[512];
                char exec_found[256] = {0};
                char name_found[64] = {0};
                char icon_found[512] = {0};
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "Exec=", 5) == 0) {
                        strncpy(exec_found, line + 5, 255);
                        char *nl = strchr(exec_found, '\n');
                        if (nl) *nl = '\0';
                        char *pct = strchr(exec_found, ' ');
                        if (pct) *pct = '\0';
                    } else if (strncmp(line, "Name=", 5) == 0) {
                        strncpy(name_found, line + 5, 63);
                        char *nl = strchr(name_found, '\n');
                        if (nl) *nl = '\0';
                    } else if (strncmp(line, "Icon=", 5) == 0) {
                        strncpy(icon_found, line + 5, 511);
                        char *nl = strchr(icon_found, '\n');
                        if (nl) *nl = '\0';
                    }
                }
                fclose(f);
                if (exec_found[0]) strncpy(ic->exec, exec_found, 255);
                if (name_found[0]) strncpy(ic->label, name_found, 63);
                if (icon_found[0]) strncpy(ic->icon_path, icon_found, 511);
            }
        } else {
            snprintf(ic->exec, sizeof(ic->exec), "xdg-open \"%s\"", full);
        }

        ic->x = ZENITH_ICON_GRID_X * col + 16;
        ic->y = ZENITH_TASKBAR_HEIGHT + ZENITH_ICON_GRID_Y * row + 16;
        col++;
        if (col >= 1) { col = 0; row++; }

        if (ic->icon_path[0] && strstr(ic->icon_path, ".png")) {
            ic->icon_surface = cairo_image_surface_create_from_png(ic->icon_path);
            if (cairo_surface_status(ic->icon_surface) != CAIRO_STATUS_SUCCESS) {
                cairo_surface_destroy(ic->icon_surface);
                ic->icon_surface = NULL;
            }
        }

        desk->icon_count++;
    }
    closedir(d);
}

static void draw_desktop(struct zenith_desktop *desk) {
    if (!desk->cr) return;
    cairo_t *cr = desk->cr;

    if (desk->wallpaper) {
        cairo_set_source_surface(cr, desk->wallpaper, 0, ZENITH_TASKBAR_HEIGHT);
        cairo_paint(cr);
    } else {
        cairo_set_source_rgb(cr, 0.07f, 0.07f, 0.07f);
        cairo_rectangle(cr, 0, ZENITH_TASKBAR_HEIGHT, desk->width, desk->height - ZENITH_TASKBAR_HEIGHT);
        cairo_fill(cr);
    }

    for (int i = 0; i < desk->icon_count; i++) {
        struct zenith_desktop_icon *ic = &desk->icons[i];

        if (ic->selected) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.3);
            cairo_rectangle(cr, ic->x - 4, ic->y - 4, ZENITH_ICON_SIZE + 8, ZENITH_ICON_SIZE + 28);
            cairo_fill(cr);
        } else if (ic->hover) {
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1);
            cairo_rectangle(cr, ic->x - 4, ic->y - 4, ZENITH_ICON_SIZE + 8, ZENITH_ICON_SIZE + 28);
            cairo_fill(cr);
        }

        if (ic->icon_surface) {
            cairo_set_source_surface(cr, ic->icon_surface, ic->x, ic->y);
            double sw = (double)ZENITH_ICON_SIZE / cairo_image_surface_get_width(ic->icon_surface);
            double sh = (double)ZENITH_ICON_SIZE / cairo_image_surface_get_height(ic->icon_surface);
            cairo_matrix_t m;
            cairo_matrix_init_scale(&m, 1.0/sw, 1.0/sh);
            cairo_pattern_t *pat = cairo_get_source(cr);
            cairo_pattern_set_matrix(pat, &m);
            cairo_rectangle(cr, ic->x, ic->y, ZENITH_ICON_SIZE, ZENITH_ICON_SIZE);
            cairo_fill(cr);
        } else {
            cairo_set_source_rgba(cr, 0.5, 0.7, 1.0, 0.9);
            cairo_rectangle(cr, ic->x + 4, ic->y + 4, ZENITH_ICON_SIZE - 8, ZENITH_ICON_SIZE - 8);
            cairo_fill(cr);
        }

        cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, ic->label, &ext);
        int tx = ic->x + (ZENITH_ICON_SIZE - (int)ext.width) / 2;
        int ty = ic->y + ZENITH_ICON_SIZE + 14;

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.6);
        cairo_rectangle(cr, tx - 2, ty - ext.height - 2, ext.width + 4, ext.height + 4);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
        cairo_move_to(cr, tx, ty);
        cairo_show_text(cr, ic->label);
    }

    cairo_surface_flush(desk->surface);
}

int zenith_desktop_init(struct zenith_server *server) {
    if (server->output_count == 0) return 0;
    struct zenith_output *out = server->outputs[0];

    struct zenith_desktop *desk = calloc(1, sizeof(*desk));
    if (!desk) return 0;
    desk->server = server;
    desk->width  = out->width;
    desk->height = out->height;

    desk->stride = desk->width * 4;
    desk->pixels = calloc(desk->stride * desk->height, 1);
    if (!desk->pixels) { free(desk); return 0; }

    desk->surface = cairo_image_surface_create_for_data(
        desk->pixels, CAIRO_FORMAT_ARGB32, desk->width, desk->height, desk->stride);
    desk->cr = cairo_create(desk->surface);

    const char *wp = server->config.wallpaper;
    if (wp && wp[0]) {
        desk->wallpaper = cairo_image_surface_create_from_png(wp);
        if (cairo_surface_status(desk->wallpaper) != CAIRO_STATUS_SUCCESS) {
            cairo_surface_destroy(desk->wallpaper);
            desk->wallpaper = NULL;
        }
    }

    load_desktop_icons(desk);
    desk->needs_redraw = 1;
    server->desktop = desk;
    draw_desktop(desk);
    return 1;
}

void zenith_desktop_render(struct zenith_desktop *desk) {
    if (!desk->needs_redraw) return;
    draw_desktop(desk);
    desk->needs_redraw = 0;
}

void zenith_desktop_reload_icons(struct zenith_desktop *desk) {
    for (int i = 0; i < desk->icon_count; i++) {
        if (desk->icons[i].icon_surface)
            cairo_surface_destroy(desk->icons[i].icon_surface);
    }
    desk->icon_count = 0;
    load_desktop_icons(desk);
    desk->needs_redraw = 1;
}

void zenith_desktop_click(struct zenith_desktop *desk, int x, int y) {
    for (int i = 0; i < desk->icon_count; i++) {
        struct zenith_desktop_icon *ic = &desk->icons[i];
        if (x >= ic->x && x < ic->x + ZENITH_ICON_SIZE &&
            y >= ic->y && y < ic->y + ZENITH_ICON_SIZE) {
            if (ic->exec[0]) {
                pid_t pid = fork();
                if (pid == 0) {
                    setsid();
                    execlp("/bin/sh", "sh", "-c", ic->exec, NULL);
                    _exit(1);
                }
            }
            return;
        }
    }
}

void zenith_desktop_destroy(struct zenith_server *server) {
    if (!server->desktop) return;
    struct zenith_desktop *desk = server->desktop;
    for (int i = 0; i < desk->icon_count; i++) {
        if (desk->icons[i].icon_surface)
            cairo_surface_destroy(desk->icons[i].icon_surface);
    }
    if (desk->wallpaper) cairo_surface_destroy(desk->wallpaper);
    if (desk->cr) cairo_destroy(desk->cr);
    if (desk->surface) cairo_surface_destroy(desk->surface);
    free(desk->pixels);
    free(desk);
    server->desktop = NULL;
}
