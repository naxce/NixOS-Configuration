project('zenithde', 'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: ['c_std=c11', 'warning_level=2'])

cc = meson.get_compiler('c')

wlroots = dependency('wlroots', version: '>=0.17.0')
wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
cairo = dependency('cairo')
pango = dependency('pango')
pangocairo = dependency('pango-cairo')
dbus = dependency('dbus-1')

src_files = ['src/zenith.c']

executable('zenithde',
  src_files,
  dependencies: [wlroots, wayland_server, xkbcommon, cairo, pango, pangocairo, dbus],
  install: true)
