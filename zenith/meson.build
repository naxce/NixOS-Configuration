project('zenith', 'c', version: '1.0.0')

add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

dep_wayland_server = dependency('wayland-server')
dep_wlroots = dependency('wlroots')
dep_cairo = dependency('cairo')
dep_pango = dependency('pango')
dep_pangocairo = dependency('pangocairo')
dep_xkbcommon = dependency('xkbcommon')
dep_dbus = dependency('dbus-1')

executable('zenith',
  'src/zenith.c',
  dependencies: [
    dep_wayland_server,
    dep_wlroots,
    dep_cairo,
    dep_pango,
    dep_pangocairo,
    dep_xkbcommon,
    dep_dbus,
  ],
  install: true
)

install_data('zenith.desktop', install_dir: 'share/wayland-sessions')