src = [
   'user_db.c',
   'user_db.h'
]

sp_user_db_lib = library('sp-user-db', src,
   include_directories : top_inc,
   dependencies : [sp_util, inih.get_variable('inih')],
   install : true
)

sp_user_db = declare_dependency(
  link_with : sp_user_db_lib,
  include_directories: [include_directories('.')],
)

pkg.generate(
  name: 'sp-user-db',
  description : 'A library to manipulate the user entries of spawny',
  libraries: sp_user_db_lib,
  version: meson.project_version()
)
