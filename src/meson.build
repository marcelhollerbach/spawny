protoc_cc = find_program('protoc-c')
protoc = dependency('libprotobuf-c')

protoc_gen = generator(protoc_cc,
  output : ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@BUILD_DIR@', '@INPUT@'])

subdir('util')

subdir('sp-protocol')
subdir('libsp_client')
subdir('sp-fallback-greeter')
subdir('sp-greeter-set')
subdir('sp-greeter-start')
subdir('sp-user-db')
subdir('sp-user-db-util')
subdir('sp-daemon')
if get_option('test-build')
  subdir('selftest')
endif
