/*
 * startmenu.c
 * Popup with: search box, app list, Shutdown / Reboot / Log out buttons.
 * Apps come from scanning .desktop files in /usr/share/applications
 * and ~/.local/share/applications.
 */

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "startmenu.h"

#define MAX_APPS 256

typedef struct {
    char name[128];
    char exec[256];
    char icon[256];
} AppEntry;

static AppEntry apps[MAX_APPS];
static int n_apps = 0;

/* Strip %f %u %F %U etc from Exec lines */
static void clean_exec(char *exec) {
    char *p = exec;
    while (*p) {
        if (*p == '%' && *(p+1)) {
            memmove(p, p+2, strlen(p+1));
        } else {
            p++;
        }
    }
    /* Trim trailing space */
    p = exec + strlen(exec) - 1;
    while (p > exec && *p == ' ') *p-- = '\0';
}

static void parse_desktop_file(const char *path) {
    if (n_apps >= MAX_APPS) return;
    FILE *f = fopen(path, "r");
    if (!f) return;

    char name[128] = "", exec[256] = "", icon[256] = "";
    bool in_entry = false;
    bool nodisplay = false;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "[Desktop Entry]") == 0) { in_entry = true; continue; }
        if (line[0] == '[') { in_entry = false; continue; }
        if (!in_entry) continue;

        char key[64], val[448];
        if (sscanf(line, "%63[^=]=%447[^\n]", key, val) != 2) continue;

        if (strcmp(key, "Name") == 0 && !name[0])
            strncpy(name, val, sizeof(name)-1);
        else if (strcmp(key, "Exec") == 0 && !exec[0])
            strncpy(exec, val, sizeof(exec)-1);
        else if (strcmp(key, "Icon") == 0 && !icon[0])
            strncpy(icon, val, sizeof(icon)-1);
        else if (strcmp(key, "NoDisplay") == 0 && strcmp(val, "true") == 0)
            nodisplay = true;
        else if (strcmp(key, "Type") == 0 && strcmp(val, "Application") != 0)
            nodisplay = true;
    }
    fclose(f);

    if (nodisplay || !name[0] || !exec[0]) return;
    clean_exec(exec);

    strncpy(apps[n_apps].name, name, sizeof(apps[n_apps].name)-1);
    strncpy(apps[n_apps].exec, exec, sizeof(apps[n_apps].exec)-1);
    strncpy(apps[n_apps].icon, icon, sizeof(apps[n_apps].icon)-1);
    n_apps++;
}

static void scan_dir(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".desktop")) continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        parse_desktop_file(path);
    }
    closedir(d);
}

static void load_apps(void) {
    n_apps = 0;
    scan_dir("/usr/share/applications");
    const char *home = getenv("HOME");
    if (home) {
        char local[512];
        snprintf(local, sizeof(local), "%s/.local/share/applications", home);
        scan_dir(local);
    }
}

/* Sort apps by name */
static int cmp_app(const void *a, const void *b) {
    return strcasecmp(((AppEntry*)a)->name, ((AppEntry*)b)->name);
}

/* ------------------------------------------------------------------ */

static GtkWidget *app_list_box = NULL;

static void launch_app(GtkButton *btn, gpointer data) {
    (void)btn;
    const char *exec = data;
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", exec, NULL);
        _exit(1);
    }
    /* Close the popover */
    GtkWidget *w = GTK_WIDGET(btn);
    while (w && !GTK_IS_POPOVER(w)) w = gtk_widget_get_parent(w);
    if (w) gtk_popover_popdown(GTK_POPOVER(w));
}

static void on_search_changed(GtkSearchEntry *entry, gpointer data) {
    (void)data;
    const char *query = gtk_editable_get_text(GTK_EDITABLE(entry));

    /* Remove all children */
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app_list_box)))
        gtk_list_box_remove(GTK_LIST_BOX(app_list_box), child);

    /* Add matching apps */
    int shown = 0;
    for (int i = 0; i < n_apps && shown < 20; i++) {
        if (query[0] && !strcasestr(apps[i].name, query)) continue;
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_margin_start(hbox, 6);
        gtk_widget_set_margin_end(hbox, 6);
        gtk_widget_set_margin_top(hbox, 4);
        gtk_widget_set_margin_bottom(hbox, 4);

        /* Icon */
        if (apps[i].icon[0]) {
            GtkWidget *img = gtk_image_new_from_icon_name(apps[i].icon);
            gtk_image_set_pixel_size(GTK_IMAGE(img), 20);
            gtk_box_append(GTK_BOX(hbox), img);
        }

        GtkWidget *btn = gtk_button_new_with_label(apps[i].name);
        gtk_button_set_has_frame(GTK_BUTTON(btn), FALSE);
        gtk_widget_set_hexpand(btn, TRUE);
        gtk_widget_set_halign(btn, GTK_ALIGN_START);
        g_signal_connect(btn, "clicked", G_CALLBACK(launch_app),
                         (gpointer)apps[i].exec);
        gtk_box_append(GTK_BOX(hbox), btn);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);
        gtk_list_box_append(GTK_LIST_BOX(app_list_box), row);
        shown++;
    }
}

static void power_action(GtkButton *btn, gpointer data) {
    (void)btn;
    const char *cmd = data;
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(1);
    }
}

void zenith_startmenu_show(GtkButton *start_btn, gpointer win) {
    load_apps();
    qsort(apps, n_apps, sizeof(AppEntry), cmp_app);

    GtkPopover *popover = GTK_POPOVER(gtk_popover_new());
    gtk_widget_set_parent(GTK_WIDGET(popover), GTK_WIDGET(start_btn));
    gtk_popover_set_position(popover, GTK_POS_BOTTOM);
    gtk_popover_set_has_arrow(popover, FALSE);
    gtk_widget_set_size_request(GTK_WIDGET(popover), 280, 420);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(vbox, 6);
    gtk_widget_set_margin_end(vbox, 6);
    gtk_widget_set_margin_top(vbox, 6);
    gtk_widget_set_margin_bottom(vbox, 6);

    /* Search */
    GtkWidget *search = gtk_search_entry_new();
    gtk_widget_set_placeholder_text(GTK_WIDGET(search), "Search apps...");
    gtk_box_append(GTK_BOX(vbox), search);

    /* Scrollable app list */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);

    app_list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(app_list_box), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app_list_box);
    gtk_box_append(GTK_BOX(vbox), scroll);

    /* Populate initial list */
    on_search_changed(GTK_SEARCH_ENTRY(search), NULL);
    g_signal_connect(search, "search-changed", G_CALLBACK(on_search_changed), NULL);

    /* Separator */
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    /* Power buttons row */
    GtkWidget *power_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(power_row, GTK_ALIGN_CENTER);

    struct { const char *label; const char *cmd; } power_btns[] = {
        { "⏻ Shutdown",  "systemctl poweroff" },
        { "↺ Reboot",    "systemctl reboot"   },
        { "⏏ Log Out",   "pkill -TERM zenithwm" },
        { NULL, NULL }
    };
    for (int i = 0; power_btns[i].label; i++) {
        GtkWidget *b = gtk_button_new_with_label(power_btns[i].label);
        gtk_widget_add_css_class(b, "taskbar-btn");
        g_signal_connect(b, "clicked", G_CALLBACK(power_action),
                         (gpointer)power_btns[i].cmd);
        gtk_box_append(GTK_BOX(power_row), b);
    }
    gtk_box_append(GTK_BOX(vbox), power_row);

    gtk_popover_set_child(popover, vbox);
    gtk_popover_popup(popover);

    (void)win;
}