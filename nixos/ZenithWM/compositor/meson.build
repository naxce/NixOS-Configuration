wlroots    = dependency('wlroots-0.18', version: '>=0.17')
wayland    = dependency('wayland-server')
xkbcommon  = dependency('xkbcommon')
pixman     = dependency('pixman-1')

compositor_src = files(
  'main.c',
  'server.c',
  'output.c',
  'input.c',
  'xdg_shell.c',
)

compositor_deps = [wlroots, wayland, xkbcommon, pixman]

# XWayland support (optional)
xwayland = dependency('xwayland', required: false)
if xwayland.found()
  compositor_deps += xwayland
  add_project_arguments('-DWLR_HAS_XWAYLAND', language: 'c')
endif

compositor_cargs = [
  '-DZENITH_DATA_DIR="' + (get_option('prefix') / get_option('datadir') / 'zenithwm') + '"',
  # wlroots internals
  '-DWLR_USE_UNSTABLE',
]

executable('zenithwm',
  compositor_src,
  dependencies: compositor_deps,
  c_args: compositor_cargs,
  install: true,
)
