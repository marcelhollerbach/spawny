project('spawny', 'c',
  default_options: [ 'c_std=gnu99', 'warning_level=2' ],
  version : '0.5.2'
)

pkg = import('pkgconfig')

config_h = configuration_data()

server_socket = '/run/spawny/spawny.service'
server_socket_debug = '/run/spawny/spawny-debug.service'
bin_dir = join_paths(get_option('prefix'), get_option('bindir'))
fallback_dir = join_paths(get_option('prefix'), get_option('libdir'), 'spawny')

config_h.set_quoted('SP_SERVER_SOCKET', server_socket)
config_h.set_quoted('SP_SERVER_DEBUG_SOCKET', server_socket_debug)
config_h.set_quoted('SP_PAM_SERVICE_GREETER', 'spawny-greeter')
config_h.set_quoted('SP_PAM_SERVICE', 'spawny')
config_h.set_quoted('SP_USER', 'spawny')
config_h.set_quoted('SP_USER_DB_DIR', '/var/lib/spawny')
config_h.set_quoted('SP_GREETER', join_paths(fallback_dir, 'greeter'))
config_h.set_quoted('SP_GREETER_FALLBACK', join_paths(fallback_dir, 'sp-fallback-greeter'))
config_h.set_quoted('PACKAGE_BIN_DIR', bin_dir)

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
