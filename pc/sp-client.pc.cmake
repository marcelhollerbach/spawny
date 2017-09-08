prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
sharedlibdir=@CMAKE_INSTALL_FULL_DATADIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: sp-client
Description: spawny client library
Version: @PROJECT_VERSION@

Requires:
Libs: -L${libdir} -L${sharedlibdir} -lSpClient
Cflags: -I${includedir}

