src = [
  'mkpath.c',
  'mkpath.h',
  'log.c',
  'log.h'
]

sp_util_lib = static_library('util', src)

sp_util = declare_dependency(
   include_directories : include_directories('.'),
   link_with : sp_util_lib
)
