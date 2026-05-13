gtk4    = dependency('gtk4')
wayland = dependency('wayland-client')

executable('zenith-monitors',
  files('main.c', 'monitor_config.c'),
  dependencies: [gtk4, wayland],
  c_args: ['-O2'],
  install: true,
)
