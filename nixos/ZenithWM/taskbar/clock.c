/* clock.c - live updating clock label */
#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include "clock.h"

static gboolean update_clock(gpointer label) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M  %a %d %b", tm);
    gtk_label_set_text(GTK_LABEL(label), buf);
    return G_SOURCE_CONTINUE;
}

GtkWidget *zenith_clock_widget_new(void) {
    GtkWidget *label = gtk_label_new("--:--");
    gtk_widget_add_css_class(label, "clock-label");
    update_clock(label);
    g_timeout_add_seconds(1, update_clock, label);
    return label;
}