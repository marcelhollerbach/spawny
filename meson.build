project('spawny', 'c',
  default_options: [ 'c_std=gnu99', 'warning_level=2' ],
  version : '0.4.99'
)

pkg = import('pkgconfig')


config_h = configuration_data()

config_h.set_quoted('SERVER_SOCKET', '/run/spawny/spawny.service')
config_h.set('SERVER_SOCKET_P', '/run/spawny/spawny.service')

config_h.set_quoted('SERVER_DEBUG_SOCKET', '/run/spawny/spawny-debug.service')

config_h.set_quoted('PAM_SERVICE_GREETER', 'spawny-greeter')
config_h.set_quoted('PAM_SERVICE', 'spawny')

config_h.set_quoted('PACKAGE_CONFIG', join_paths(get_option('prefix'), get_option('sysconfdir'), 'spawny.ini'))

config_h.set_quoted('PACKAGE_BIN_DIR', join_paths(get_option('prefix'), get_option('bindir')))
config_h.set('PACKAGE_BIN_DIR_P', join_paths(get_option('prefix'), get_option('bindir')))

config_h.set_quoted('USER', 'spawny')
config_h.set_quoted('USER_DB_DIR', '/var/lib/spawny')
config_h.set_quoted('GREETER_BINARY_PATH', join_paths(get_option('prefix'),get_option('libdir'),'spawny','greeter'))

top_inc = include_directories('.')

inih = subproject('inih')

subdir('src')
subdir('data')
subdir('src/tests')

configure_file(
  output : 'config.h',
  install : false,
  configuration: config_h
)
