#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <cairo.h>
#include <dbus/dbus.h>
#include <dirent.h>
#include <libxkbcommon/xkbcommon.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

struct desktop_icon {
	char *name;
	char *exec;
	int x, y;
	int w, h;
};

struct menu_entry {
	char *name;
	char *exec;
};

struct keybinding {
	xkb_keysym_t sym;
	uint32_t mod;
	void (*action)(void *data);
};

struct zenith_server {
	struct wl_display *display;
	struct wlr_backend *backend;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_scene *scene;
	struct wlr_scene_tree *layers[NB_LAYER_SHELL_LAYERS];
	struct wlr_scene_tree *drag_icon;
	struct wlr_xdg_shell *xdg_shell;
	struct wlr_layer_shell_v1 *layer_shell;
	struct wlr_cursor *cursor;
	struct wlr_xcursor_manager *cursor_mgr;
	struct wlr_seat *seat;
	struct wlr_output_layout *output_layout;
	struct wl_listener new_output;
	struct wl_listener new_xdg_surface;
	struct wl_listener new_layer_surface;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;
	struct wl_listener seat_request_cursor;
	struct wl_listener seat_request_set_selection;
	struct wl_listener keyboard_key;
	struct wl_listener keyboard_modifiers;

	struct wlr_output *active_output;
	struct wlr_scene_buffer *panel_buf;
	cairo_surface_t *panel_csurf;
	cairo_t *panel_cr;
	int panel_height;
	struct wlr_box panel_geo;
	struct wlr_scene_tree *panel_tree;
	uint32_t panel_width;

	bool start_menu_visible;
	struct wlr_scene_buffer *menu_buf;
	cairo_surface_t *menu_csurf;
	cairo_t *menu_cr;
	struct wlr_box menu_geo;
	char search_text[256];
	struct menu_entry *menu_entries;
	int menu_entry_count;
	int menu_scroll;
	int menu_selected;

	bool settings_visible;
	struct wlr_scene_buffer *settings_buf;
	cairo_surface_t *settings_csurf;
	cairo_t *settings_cr;
	struct wlr_box settings_geo;
	int settings_active_output_idx;
	int settings_mode_idx;
	int settings_new_x, settings_new_y;
	float settings_new_scale;
	int settings_new_refresh;

	struct desktop_icon *icons;
	int icon_count;

	struct keybinding *keybindings;
	int keybinding_count;

	char config_path[512];
	char monitors_path[512];
};

enum {
	LAYER_BACKGROUND,
	LAYER_BOTTOM,
	LAYER_TOP,
	LAYER_OVERLAY,
};

static void spawn(const char *cmd) {
	if (fork() == 0) {
		setsid();
		execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
		_exit(1);
	}
}

static void shutdown_dbus(const char *method) {
	DBusError err;
	dbus_error_init(&err);
	DBusConnection *conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn) return;
	DBusMessage *msg = dbus_message_new_method_call(
		"org.freedesktop.login1", "/org/freedesktop/login1",
		"org.freedesktop.login1.Manager", method);
	if (!msg) return;
	dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, &(dbus_bool_t){1}, DBUS_TYPE_INVALID);
	dbus_connection_send(conn, msg, NULL);
	dbus_message_unref(msg);
	dbus_connection_unref(conn);
}

static void action_shutdown(void *data) { shutdown_dbus("PowerOff"); }
static void action_reboot(void *data)   { shutdown_dbus("Reboot"); }
static void action_logout(void *data)   { 
	struct zenith_server *server = data;
	wl_display_terminate(server->display);
}

static void action_terminal(void *data) { spawn("x-terminal-emulator || foot || alacritty"); }
static void action_close(void *data) {
	struct zenith_server *server = data;
	struct wlr_xdg_toplevel *toplevel = NULL;
	struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->tree.node,
		server->cursor->x, server->cursor->y, NULL, NULL);
	if (node) {
		struct wlr_scene_tree *tree = NULL;
		if (node->type == WLR_SCENE_NODE_BUFFER) {
			struct wlr_scene_buffer *buf = wlr_scene_buffer_from_node(node);
			tree = buf->node.parent;
		}
		if (tree && tree->node.data)
			toplevel = tree->node.data;
	}
	if (toplevel)
		wlr_xdg_toplevel_send_close(toplevel);
}
static void action_toggle_fullscreen(void *data) {
	struct zenith_server *server = data;
	struct wlr_xdg_toplevel *toplevel = NULL;
	struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->tree.node,
		server->cursor->x, server->cursor->y, NULL, NULL);
	if (node) {
		struct wlr_scene_tree *tree = NULL;
		if (node->type == WLR_SCENE_NODE_BUFFER) {
			struct wlr_scene_buffer *buf = wlr_scene_buffer_from_node(node);
			tree = buf->node.parent;
		}
		if (tree && tree->node.data)
			toplevel = tree->node.data;
	}
	if (toplevel) {
		if (toplevel->requested.fullscreen)
			wlr_xdg_toplevel_set_fullscreen(toplevel, false);
		else
			wlr_xdg_toplevel_set_fullscreen(toplevel, true);
	}
}
static void action_minimize_all(void *data) {
	struct zenith_server *server = data;
	struct wlr_scene_node *node;
	wl_list_for_each(node, &server->scene->tree.children, link) {
		if (node->type == WLR_SCENE_NODE_TREE && node->data) {
			struct wlr_xdg_toplevel *t = node->data;
			wlr_xdg_toplevel_set_minimized(t, true);
		}
	}
}

static void keybinding_execute(struct zenith_server *server, xkb_keysym_t sym, uint32_t mods) {
	for (int i = 0; i < server->keybinding_count; i++) {
		struct keybinding *kb = &server->keybindings[i];
		if (kb->sym == sym && kb->mod == mods) {
			kb->action(server);
			return;
		}
	}
}

static void config_load(struct zenith_server *server);
static void config_save_monitors(struct zenith_server *server);

static void panel_draw(struct zenith_server *server);
static void menu_draw(struct zenith_server *server);
static void settings_draw(struct zenith_server *server);

static void panel_motion(struct zenith_server *server, double x, double y);
static void panel_button(struct zenith_server *server, double x, double y, uint32_t state);
static void menu_button(struct zenith_server *server, double x, double y, uint32_t state);
static void settings_button(struct zenith_server *server, double x, double y, uint32_t state);

static void menu_add_search(struct zenith_server *server, const char *text);
static void menu_commit_search(struct zenith_server *server);

static void desktop_scan(struct zenith_server *server);
static void desktop_render(cairo_t *cr, struct zenith_server *server);

static void start_menu_toggle(struct zenith_server *server) {
	server->start_menu_visible = !server->start_menu_visible;
	if (server->start_menu_visible) {
		memset(server->search_text, 0, sizeof(server->search_text));
		menu_add_search(server, "");
		struct wlr_box panel = server->panel_geo;
		server->menu_geo.x = panel.x + 5;
		server->menu_geo.y = panel.y + panel.height;
		server->menu_geo.width = 400;
		server->menu_geo.height = 400;
	} else {
		server->menu_geo.width = 0;
		server->menu_geo.height = 0;
	}
	menu_draw(server);
}

static void settings_toggle(struct zenith_server *server) {
	server->settings_visible = !server->settings_visible;
	if (server->settings_visible) {
		struct wlr_output *out = server->active_output;
		if (out) {
			server->settings_active_output_idx = 0;
			server->settings_new_x = out->lx;
			server->settings_new_y = out->ly;
			server->settings_new_scale = out->scale;
			server->settings_new_refresh = out->current_mode ? out->current_mode->refresh : 60000;
			server->settings_geo.x = 0;
			server->settings_geo.y = 0;
			server->settings_geo.width = out->width;
			server->settings_geo.height = out->height;
		}
	}
	settings_draw(server);
}

static void focus_window(struct zenith_server *server, struct wlr_xdg_toplevel *toplevel) {
	struct wlr_scene_tree *tree = toplevel->base->data;
	wlr_scene_node_raise_to_top(&tree->node);
	struct wlr_keyboard *kb = wlr_seat_get_keyboard(server->seat);
	if (kb) {
		wlr_seat_keyboard_notify_enter(server->seat, toplevel->base->surface,
			kb->keycodes, kb->num_keycodes, &kb->modifiers);
	}
}

static void xdg_surface_map(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
		struct wlr_scene_tree *tree = wlr_scene_xdg_surface_create(server->layers[LAYER_TOP], xdg_surface);
		toplevel->base->data = tree;
		tree->node.data = toplevel;
	}
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data) {
	struct wlr_xdg_surface *xdg_surface = data;
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		struct wlr_scene_tree *tree = xdg_surface->data;
		if (tree) tree->node.data = NULL;
	}
}

static void layer_surface_map(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, new_layer_surface);
	struct wlr_layer_surface_v1 *surface = data;
	struct wlr_scene_tree *parent = server->layers[surface->current.layer];
	struct wlr_scene_layer_surface_v1 *scene_surface = wlr_scene_layer_surface_v1_create(parent, surface);
	scene_surface->tree->node.data = surface;
}

static void layer_surface_unmap(struct wl_listener *listener, void *data) {
	struct wlr_layer_surface_v1 *surface = data;
	struct wlr_scene_layer_surface_v1 *scene_surface = surface->data;
	if (scene_surface) {
		wlr_scene_node_destroy(&scene_surface->tree->node);
		surface->data = NULL;
	}
}

static void output_frame(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, new_output);
	struct wlr_output *output = data;
	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(server->scene, output);
	if (!scene_output) return;
	wlr_scene_output_commit(scene_output);
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}

static void new_output(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, new_output);
	struct wlr_output *output = data;
	wlr_output_layout_add_auto(server->output_layout, output);
	struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene, output);
	wl_listener_init(&output->frame, output_frame);
	wl_signal_add(&output->events.frame, &output->frame);
	if (!server->active_output) server->active_output = output;
	config_save_monitors(server);
}

static void cursor_motion_abs(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_pointer_motion_absolute_event *event = data;
	wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x, event->y);
	wlr_seat_pointer_notify_frame(server->seat);
}

static void cursor_motion_rel(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, cursor_motion);
	struct wlr_pointer_motion_event *event = data;
	wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x, event->delta_y);
	wlr_seat_pointer_notify_frame(server->seat);
}

static void cursor_button(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, cursor_button);
	struct wlr_pointer_button_event *event = data;
	wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_surface *surface = NULL;
	struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->tree.node,
		server->cursor->x, server->cursor->y, &sx, &sy);
	if (node) {
		if (node->type == WLR_SCENE_NODE_BUFFER) {
			struct wlr_scene_buffer *buf = wlr_scene_buffer_from_node(node);
			surface = wlr_scene_buffer_get_surface(buf);
		}
	}
	if (event->state == WLR_BUTTON_PRESSED) {
		if (server->panel_tree) {
			struct wlr_box pbox = server->panel_geo;
			if (server->cursor->x >= pbox.x && server->cursor->x < pbox.x + pbox.width &&
			    server->cursor->y >= pbox.y && server->cursor->y < pbox.y + pbox.height) {
				panel_button(server, server->cursor->x - pbox.x, server->cursor->y - pbox.y, event->state);
				return;
			}
		}
		if (server->start_menu_visible) {
			struct wlr_box m = server->menu_geo;
			if (server->cursor->x >= m.x && server->cursor->x < m.x + m.width &&
			    server->cursor->y >= m.y && server->cursor->y < m.y + m.height) {
				menu_button(server, server->cursor->x - m.x, server->cursor->y - m.y, event->state);
				return;
			} else start_menu_toggle(server);
		}
		if (server->settings_visible) {
			struct wlr_box s = server->settings_geo;
			if (server->cursor->x >= s.x && server->cursor->x < s.x + s.width &&
			    server->cursor->y >= s.y && server->cursor->y < s.y + s.height) {
				settings_button(server, server->cursor->x - s.x, server->cursor->y - s.y, event->state);
				return;
			} else settings_toggle(server);
		}
		if (surface) {
			struct wlr_xdg_surface *xdg = wlr_xdg_surface_try_from_wlr_surface(surface);
			if (xdg && xdg->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
				focus_window(server, xdg->toplevel);
		}
	}
}

static void keyboard_key(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, keyboard_key);
	struct wlr_keyboard_key_event *event = data;
	struct wlr_keyboard *kb = event->keyboard;
	uint32_t mods = wlr_keyboard_get_modifiers(kb);
	xkb_keysym_t sym = xkb_state_key_get_one_sym(kb->xkb_state, event->keycode + 8);
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED)
		keybinding_execute(server, sym, mods);
	wlr_seat_set_keyboard(server->seat, kb);
	wlr_seat_keyboard_notify_key(server->seat, event->time_msec, event->keycode, event->state);
}

static void keyboard_modifiers(struct wl_listener *listener, void *data) {
	struct zenith_server *server = wl_container_of(listener, server, keyboard_modifiers);
	wlr_seat_set_keyboard(server->seat, data);
	wlr_seat_keyboard_notify_modifiers(server->seat, &((struct wlr_keyboard *)data)->modifiers);
}

int main(int argc, char *argv[]) {
	struct zenith_server server = {0};
	server.display = wl_display_create();
	server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.display), NULL);
	if (!server.backend) return 1;

	server.renderer = wlr_renderer_autocreate(server.backend);
	wlr_renderer_init_wl_shm(server.renderer, server.display);
	server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
	server.scene = wlr_scene_create();

	server.output_layout = wlr_output_layout_create();
	wlr_scene_attach_output_layout(server.scene, server.output_layout);
	for (int i = 0; i < 4; i++) server.layers[i] = wlr_scene_tree_create(&server.scene->tree);

	server.xdg_shell = wlr_xdg_shell_create(server.display, 3);
	server.layer_shell = wlr_layer_shell_v1_create(server.display, 3);

	server.cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(server.cursor, server.output_layout);
	server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(server.cursor_mgr, 1);

	server.seat = wlr_seat_create(server.display, "seat0");

	struct wlr_compositor *compositor = wlr_compositor_create(server.display, 5, server.renderer);
	wlr_subcompositor_create(server.display);
	wlr_data_device_manager_create(server.display);

	wl_listener_init(&server.new_output, new_output);
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

	wl_listener_init(&server.new_xdg_surface, xdg_surface_map);
	wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_xdg_surface);
	wl_listener_init(&server.new_xdg_surface, xdg_surface_unmap);
	wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_xdg_surface);

	wl_listener_init(&server.new_layer_surface, layer_surface_map);
	wl_signal_add(&server.layer_shell->events.new_surface, &server.new_layer_surface);
	wl_listener_init(&server.new_layer_surface, layer_surface_unmap);
	wl_signal_add(&server.layer_shell->events.new_surface, &server.new_layer_surface);

	wl_listener_init(&server.cursor_motion, cursor_motion_rel);
	wl_signal_add(&server.cursor->events.motion, &server.cursor_motion);
	wl_listener_init(&server.cursor_motion_absolute, cursor_motion_abs);
	wl_signal_add(&server.cursor->events.motion_absolute, &server.cursor_motion_absolute);
	wl_listener_init(&server.cursor_button, cursor_button);
	wl_signal_add(&server.cursor->events.button, &server.cursor_button);
	wl_listener_init(&server.cursor_axis, NULL);

	wl_listener_init(&server.keyboard_key, keyboard_key);
	wl_signal_add(&server.seat->events.keyboard_key, &server.keyboard_key);
	wl_listener_init(&server.keyboard_modifiers, keyboard_modifiers);
	wl_signal_add(&server.seat->events.keyboard_modifiers, &server.keyboard_modifiers);

	const char *home = getenv("HOME");
	snprintf(server.config_path, sizeof(server.config_path), "%s/.config/zenith", home);
	mkdir(server.config_path, 0755);
	snprintf(server.monitors_path, sizeof(server.monitors_path), "%s/monitors.conf", server.config_path);
	config_load(&server);
	desktop_scan(&server);

	server.keybindings = calloc(5, sizeof(struct keybinding));
	server.keybindings[0] = (struct keybinding){ XKB_KEY_Return, WLR_MODIFIER_LOGO, action_terminal };
	server.keybindings[1] = (struct keybinding){ XKB_KEY_q, WLR_MODIFIER_LOGO, action_close };
	server.keybindings[2] = (struct keybinding){ XKB_KEY_f, WLR_MODIFIER_LOGO, action_toggle_fullscreen };
	server.keybindings[3] = (struct keybinding){ XKB_KEY_m, WLR_MODIFIER_LOGO, action_minimize_all };
	server.keybindings[4] = (struct keybinding){ XKB_KEY_d, WLR_MODIFIER_LOGO, start_menu_toggle };
	server.keybinding_count = 5;

	wlr_backend_start(server.backend);
	wl_display_run(server.display);

	wl_display_destroy(server.display);
	return 0;
}

static void config_load(struct zenith_server *server) {
	server->panel_height = 32;
}

static void config_save_monitors(struct zenith_server *server) {
	FILE *f = fopen(server->monitors_path, "w");
	if (!f) return;
	struct wlr_output *output;
	wl_list_for_each(output, &server->backend->outputs, link) {
		fprintf(f, "%s %d %d %f %d\n",
			output->name, output->lx, output->ly, output->scale,
			output->current_mode ? output->current_mode->refresh : 60000);
	}
	fclose(f);
}

static void desktop_scan(struct zenith_server *server) {
	char desk[512];
	snprintf(desk, sizeof(desk), "%s/Desktop", getenv("HOME"));
	DIR *d = opendir(desk);
	if (!d) return;
	server->icon_count = 0;
	server->icons = NULL;
	struct dirent *ent;
	while ((ent = readdir(d))) {
		if (!strstr(ent->d_name, ".desktop")) continue;
		char path[1024];
		snprintf(path, sizeof(path), "%s/%s", desk, ent->d_name);
		FILE *f = fopen(path, "r");
		if (!f) continue;
		char line[256], name[128]="", exec[256]="";
		while (fgets(line, sizeof(line), f)) {
			if (strncmp(line, "Name=", 5) == 0) { strncpy(name, line+5, 127); name[strcspn(name, "\n")]=0; }
			if (strncmp(line, "Exec=", 5) == 0) { strncpy(exec, line+5, 255); exec[strcspn(exec, "\n")]=0; }
		}
		fclose(f);
		if (name[0] && exec[0]) {
			server->icons = realloc(server->icons, (server->icon_count+1)*sizeof(struct desktop_icon));
			struct desktop_icon *ic = &server->icons[server->icon_count++];
			ic->name = strdup(name);
			ic->exec = strdup(exec);
		}
	}
	closedir(d);
}

static void panel_draw(struct zenith_server *server) {
	if (!server->panel_cr) return;
	cairo_t *cr = server->panel_cr;
	int w = server->panel_width, h = server->panel_height;
	cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
	cairo_rectangle(cr, 0, 0, w, h);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 14);
	cairo_move_to(cr, 10, 22);
	cairo_show_text(cr, "Zenith");

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char timestr[64];
	strftime(timestr, sizeof(timestr), "%H:%M", tm);
	cairo_text_extents_t ext;
	cairo_text_extents(cr, timestr, &ext);
	cairo_move_to(cr, w - ext.width - 120, 22);
	cairo_show_text(cr, timestr);

	cairo_move_to(cr, w - 100, 22);
	cairo_show_text(cr, "_");
	cairo_move_to(cr, w - 60, 22);
	cairo_show_text(cr, "\u2699");

	wlr_scene_buffer_set_buffer(server->panel_buf, &(struct wlr_buffer){
		.width = w, .height = h, .data = cairo_image_surface_get_data(server->panel_csurf) });
}

static void menu_draw(struct zenith_server *server) {
	if (!server->menu_cr) return;
	cairo_t *cr = server->menu_cr;
	int w = server->menu_geo.width, h = server->menu_geo.height;
	cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
	cairo_rectangle(cr, 0, 0, w, h);
	cairo_fill(cr);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, "TODO menu", -1);
	cairo_move_to(cr, 10, 10);
	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);
	wlr_scene_buffer_set_buffer(server->menu_buf, &(struct wlr_buffer){
		.width = w, .height = h, .data = cairo_image_surface_get_data(server->menu_csurf) });
}

static void settings_draw(struct zenith_server *server) { /* simplified */ }

static void panel_button(struct zenith_server *server, double x, double y, uint32_t state) {
	int w = server->panel_width;
	if (x >= w-100 && x < w-60) { action_minimize_all(server); return; }
	if (x >= w-60 && x < w-20) { settings_toggle(server); return; }
	if (x >= 10 && x < 80) start_menu_toggle(server);
}

static void menu_button(struct zenith_server *server, double x, double y, uint32_t state) {}
static void settings_button(struct zenith_server *server, double x, double y, uint32_t state) {}
static void menu_add_search(struct zenith_server *server, const char *text) {}
static void menu_commit_search(struct zenith_server *server) {}