fallback_dir = join_paths(get_option('prefix'), get_option('libdir'), 'spawny')

sp_fallback_greeter = executable('sp-fallback-greeter', 'main.c',
  dependencies : sp_client,
  install : true,
  install_dir : fallback_dir
)

config_h.set_quoted('FALLBACK_GREETER', join_paths(fallback_dir, 'sp-fallback-greeter'))
