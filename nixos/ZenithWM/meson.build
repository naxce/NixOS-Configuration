project('zenithwm', 'c',
  version: '0.1.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'optimization=3',         # -O3 for performance
    'b_lto=true',             # link-time optimization
  ]
)

subdir('compositor')
subdir('taskbar')
subdir('monitor-config')

# Install config defaults
install_data(
  'config/zenithwm.conf',
  'config/keys.conf',
  'config/desktop.conf',
  install_dir: get_option('datadir') / 'zenithwm' / 'defaults'
)

# Install session files
install_data(
  'session/zenithwm-session',
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x'
)

install_data(
  'session/zenithwm.desktop',
  install_dir: get_option('datadir') / 'wayland-sessions'
)

install_data(
  'sddm/zenithwm.desktop',
  install_dir: get_option('datadir') / 'wayland-sessions'
)
