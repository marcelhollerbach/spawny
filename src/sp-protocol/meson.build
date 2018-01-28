server_proto = custom_target('server_proto-gen',
   input : ['server.proto', 'greeter.proto'],
   output : ['server.pb-c.c','server.pb-c.h', 'greeter.pb-c.c','greeter.pb-c.h'],
   command : [protoc_cc, '--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@OUTDIR@', '@INPUT@']
)

sp_protocol_lib = static_library('sp-protocol', server_proto,
  ['socket_creation.c'],
  include_directories: [top_inc, include_directories('.')],
  dependencies : protoc
)

sp_protocol = declare_dependency(
  link_with : sp_protocol_lib,
  dependencies : protoc,
  sources : server_proto,
  include_directories: [top_inc, include_directories('.')],
)
