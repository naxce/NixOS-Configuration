gtk4        = dependency('gtk4')
layer_shell = dependency('gtk4-layer-shell-0')

taskbar_src = files(
  'main.c',
  'taskbar.c',
  'clock.c',
  'startmenu.c',
  'settings_win.c',
)

executable('zenith-taskbar',
  taskbar_src,
  dependencies: [gtk4, layer_shell],
  c_args: ['-O3'],
  install: true,
)
