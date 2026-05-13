#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <linux/input-event-codes.h>

#include <cairo/cairo.h>

#include "zenith.h"
#include "launcher.h"
#include "output.h"

static void parse_desktop_file(const char *path, struct launcher_entry *e) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    int in_entry = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "[Desktop Entry]", 15) == 0) { in_entry = 1; continue; }
        if (line[0] == '[' && in_entry) break;
        if (!in_entry) continue;
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (strncmp(line, "Name=", 5) == 0 && !e->name[0])
            strncpy(e->name, line + 5, 127);
        else if (strncmp(line, "Exec=", 5) == 0 && !e->exec[0]) {
            strncpy(e->exec, line + 5, 255);
            char *pct = strstr(e->exec, " %");
            if (pct) *pct = '\0';
        } else if (strncmp(line, "Icon=", 5) == 0 && !e->icon[0])
            strncpy(e->icon, line + 5, 511);
        else if (strncmp(line, "Comment=", 8) == 0 && !e->comment[0])
            strncpy(e->comment, line + 8, 255);
        else if (strncmp(line, "NoDisplay=true", 14) == 0) {
            fclose(f);
            memset(e, 0, sizeof(*e));
            return;
        }
    }
    fclose(f);
}

void zenith_launcher_scan_apps(struct zenith_launcher *launcher) {
    launcher->app_count = 0;
    static const char *dirs[] = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        NULL
    };
    char home_apps[PATH_MAX];
    const char *home = getenv("HOME");
    if (home) snprintf(home_apps, sizeof(home_apps), "%s/.local/share/applications", home);

    for (int d = 0; d < 3; d++) {
        const char *dir = (d < 2) ? dirs[d] : (home ? home_apps : NULL);
        if (!dir) continue;
        DIR *dp = opendir(dir);
        if (!dp) continue;
        struct dirent *ent;
        while ((ent = readdir(dp)) != NULL && launcher->app_count < 512) {
            if (!strstr(ent->d_name, ".desktop")) continue;
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
            struct launcher_entry e = {0};
            parse_desktop_file(path, &e);
            if (e.name[0] && e.exec[0]) {
                launcher->all_apps[launcher->app_count++] = e;
            }
        }
        closedir(dp);
    }
}

static void update_results(struct zenith_launcher *l) {
    l->result_count = 0;
    l->selected = 0;
    for (int i = 0; i < l->app_count && l->result_count < ZENITH_LAUNCHER_MAX_RESULTS; i++) {
        if (!l->query[0] ||
            strcasestr(l->all_apps[i].name, l->query) ||
            strcasestr(l->all_apps[i].exec, l->query)) {
            l->results[l->result_count++] = l->all_apps[i];
        }
    }
}

static void draw_launcher(struct zenith_launcher *l) {
    if (!l->cr) return;
    cairo_t *cr = l->cr;

    cairo_set_source_rgba(cr, 0.05, 0.05, 0.05, 0.97);
    cairo_rectangle(cr, 0, 0, l->w, l->h);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.8);
    cairo_rectangle(cr, 0, 0, l->w, 1);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, 0, 1, l->h);
    cairo_fill(cr);
    cairo_rectangle(cr, l->w - 1, 0, 1, l->h);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, l->h - 1, l->w, 1);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0.12, 0.12, 0.12, 1.0);
    cairo_rectangle(cr, 8, 8, l->w - 16, 32);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.6);
    cairo_rectangle(cr, 8, 8, l->w - 16, 1);
    cairo_fill(cr);
    cairo_rectangle(cr, 8, 39, l->w - 16, 1);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 13);
    char display_query[260];
    snprintf(display_query, sizeof(display_query), "> %s_", l->query);
    cairo_move_to(cr, 14, 30);
    cairo_show_text(cr, display_query);

    static const char *power_labels[] = {"Shutdown", "Reboot", "Logout"};
    static const char *power_cmds[]   = {
        "systemctl poweroff",
        "systemctl reboot",
        "loginctl terminate-session $XDG_SESSION_ID"
    };
    int pw = (l->w - 16 - 8) / 3;
    for (int i = 0; i < 3; i++) {
        int bx = 8 + i * (pw + 4);
        cairo_set_source_rgba(cr, 0.15, 0.05, 0.05, 1.0);
        if (i == 2) cairo_set_source_rgba(cr, 0.05, 0.05, 0.15, 1.0);
        cairo_rectangle(cr, bx, l->h - 36, pw, 28);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.85, 0.3, 0.3);
        if (i == 2) cairo_set_source_rgb(cr, 0.3, 0.6, 0.85);
        cairo_set_font_size(cr, 11);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, power_labels[i], &ext);
        cairo_move_to(cr, bx + (pw - ext.width) / 2.0, l->h - 36 + 18);
        cairo_show_text(cr, power_labels[i]);
        (void)power_cmds;
    }

    int item_h = 32;
    int list_y = 48;
    for (int i = 0; i < l->result_count; i++) {
        int iy = list_y + i * item_h;
        if (iy + item_h > l->h - 44) break;
        if (i == l->selected) {
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.25);
            cairo_rectangle(cr, 4, iy, l->w - 8, item_h - 2);
            cairo_fill(cr);
            cairo_set_source_rgba(cr, 0.27, 0.53, 1.0, 0.6);
            cairo_rectangle(cr, 4, iy, 2, item_h - 2);
            cairo_fill(cr);
        }

        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        cairo_move_to(cr, 14, iy + 18);
        cairo_show_text(cr, l->results[i].name);

        if (l->results[i].comment[0]) {
            cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
            cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, 10);
            cairo_move_to(cr, 14, iy + 29);
            cairo_show_text(cr, l->results[i].comment);
        }
    }

    cairo_surface_flush(l->surface);
}

int zenith_launcher_init(struct zenith_server *server) {
    struct zenith_launcher *l = calloc(1, sizeof(*l));
    if (!l) return 0;
    l->server = server;

    int ow = 800, oh = 600;
    if (server->output_count > 0) {
        ow = server->outputs[0]->width;
        oh = server->outputs[0]->height;
    }

    l->w = ZENITH_LAUNCHER_WIDTH;
    l->h = ZENITH_LAUNCHER_HEIGHT;
    l->x = (ow - l->w) / 2;
    l->y = (oh - l->h) / 3;

    l->stride = l->w * 4;
    l->pixels = calloc(l->stride * l->h, 1);
    l->surface = cairo_image_surface_create_for_data(
        l->pixels, CAIRO_FORMAT_ARGB32, l->w, l->h, l->stride);
    l->cr = cairo_create(l->surface);

    zenith_launcher_scan_apps(l);
    update_results(l);
    l->visible = 0;
    l->needs_redraw = 1;
    server->launcher = l;
    return 1;
}

void zenith_launcher_toggle(struct zenith_launcher *l) {
    if (!l) return;
    l->visible = !l->visible;
    if (l->visible) {
        memset(l->query, 0, sizeof(l->query));
        l->query_len = 0;
        zenith_launcher_scan_apps(l);
        update_results(l);
        l->needs_redraw = 1;
    }
}

void zenith_launcher_key(struct zenith_launcher *l, unsigned int key, unsigned int mods) {
    (void)mods;
    if (!l->visible) return;

    if (key == KEY_ESC) {
        l->visible = 0;
        return;
    }
    if (key == KEY_ENTER) {
        if (l->selected < l->result_count && l->results[l->selected].exec[0]) {
            pid_t pid = fork();
            if (pid == 0) {
                setsid();
                execlp("/bin/sh", "sh", "-c", l->results[l->selected].exec, NULL);
                _exit(1);
            }
            l->visible = 0;
        }
        return;
    }
    if (key == KEY_UP) {
        if (l->selected > 0) l->selected--;
        l->needs_redraw = 1;
        return;
    }
    if (key == KEY_DOWN) {
        if (l->selected < l->result_count - 1) l->selected++;
        l->needs_redraw = 1;
        return;
    }
    if (key == KEY_BACKSPACE) {
        if (l->query_len > 0) {
            l->query[--l->query_len] = '\0';
            update_results(l);
            l->needs_redraw = 1;
        }
        return;
    }

    if (key >= KEY_A && key <= KEY_Z) {
        char c = 'a' + (key - KEY_A);
        if (mods & (1 << 0)) c -= 32;
        if (l->query_len < 254) {
            l->query[l->query_len++] = c;
            l->query[l->query_len] = '\0';
            update_results(l);
            l->needs_redraw = 1;
        }
    } else if (key >= KEY_1 && key <= KEY_9) {
        char c = '1' + (key - KEY_1);
        if (l->query_len < 254) {
            l->query[l->query_len++] = c;
            l->query[l->query_len] = '\0';
            update_results(l);
            l->needs_redraw = 1;
        }
    } else if (key == KEY_0) {
        if (l->query_len < 254) {
            l->query[l->query_len++] = '0';
            l->query[l->query_len] = '\0';
            update_results(l);
            l->needs_redraw = 1;
        }
    } else if (key == KEY_SPACE) {
        if (l->query_len < 254) {
            l->query[l->query_len++] = ' ';
            l->query[l->query_len] = '\0';
            update_results(l);
            l->needs_redraw = 1;
        }
    }
}

void zenith_launcher_click(struct zenith_launcher *l, int x, int y) {
    if (!l->visible) return;
    int rx = x - l->x;
    int ry = y - l->y;
    if (rx < 0 || rx >= l->w || ry < 0 || ry >= l->h) {
        l->visible = 0;
        return;
    }

    int pw = (l->w - 16 - 8) / 3;
    if (ry >= l->h - 36 && ry < l->h - 8) {
        static const char *cmds[] = {
            "systemctl poweroff",
            "systemctl reboot",
            "loginctl terminate-session $XDG_SESSION_ID"
        };
        for (int i = 0; i < 3; i++) {
            int bx = 8 + i * (pw + 4);
            if (rx >= bx && rx < bx + pw) {
                pid_t pid = fork();
                if (pid == 0) {
                    setsid();
                    execlp("/bin/sh", "sh", "-c", cmds[i], NULL);
                    _exit(1);
                }
                l->visible = 0;
                return;
            }
        }
    }

    int item_h = 32;
    int list_y = 48;
    for (int i = 0; i < l->result_count; i++) {
        int iy = list_y + i * item_h;
        if (iy + item_h > l->h - 44) break;
        if (rx >= 4 && rx < l->w - 4 && ry >= iy && ry < iy + item_h) {
            l->selected = i;
            if (l->results[i].exec[0]) {
                pid_t pid = fork();
                if (pid == 0) {
                    setsid();
                    execlp("/bin/sh", "sh", "-c", l->results[i].exec, NULL);
                    _exit(1);
                }
                l->visible = 0;
            }
            return;
        }
    }
}

void zenith_launcher_render(struct zenith_launcher *l) {
    if (!l->visible || !l->needs_redraw) return;
    draw_launcher(l);
    l->needs_redraw = 0;
}

void zenith_launcher_destroy(struct zenith_server *server) {
    if (!server->launcher) return;
    struct zenith_launcher *l = server->launcher;
    if (l->cr) cairo_destroy(l->cr);
    if (l->surface) cairo_surface_destroy(l->surface);
    free(l->pixels);
    free(l);
    server->launcher = NULL;
}
