src = [
  'array.c',
  'array.h',
  'desktop_common.c',
  'desktop_common.h',
  'greeter.c',
  'greeter.h',
  'main.c',
  'main.h',
  'manager.c',
  'manager.h',
  'server.c',
  'server.h',
  'server_systemd.c',
  'sessionmgt.h',
  'sessionmgt_logind.c',
  'spawnregistery.c',
  'spawnregistery.h',
  'spawnservice.c',
  'spawnservice.h',
  'templatereg.c',
  'templatereg.h',
  'templates.h',
  'tty_temp.c',
  'wl_temp.c',
  'x11_temp.c',
]

session_protocol = protoc_gen.process('session_spawn.proto')

cc = meson.get_compiler('c')

systemd = dependency('libsystemd')
pam = cc.find_library('pam')
m = cc.find_library('m')

sp_daemon = executable('sp-daemon', src, session_protocol,
  dependencies: [sp_protocol, inih.get_variable('inih'), sp_user_db, sp_client, sp_util, systemd, pam, m],
  install : true
)
