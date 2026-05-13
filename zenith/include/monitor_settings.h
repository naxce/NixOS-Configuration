#ifndef ZENITH_MONITOR_SETTINGS_H
#define ZENITH_MONITOR_SETTINGS_H

struct zenith_server;

void zenith_monitor_settings_apply(struct zenith_server *server);
void zenith_monitor_settings_save(struct zenith_server *server);
void zenith_monitor_settings_load(struct zenith_server *server);

#endif
