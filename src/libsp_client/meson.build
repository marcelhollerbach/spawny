sp_client_lib = library('SpClient',
  ['client.c'],
  dependencies : [sp_protocol, sp_util],
  install : true
)

sp_client = declare_dependency(
  link_with : sp_client_lib,
  dependencies : [sp_protocol],
  include_directories: [include_directories('.')]
)

pkg.generate(
  name: 'sp-client',
  description : 'The client library for communicating with sp-daemon',
  libraries: sp_client_lib,
  version: meson.project_version()
)
