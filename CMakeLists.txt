cmake_minimum_required (VERSION 2.6)
project (Spawny)

include(GNUInstallDirs)
enable_testing()

add_definitions("-Wall -O0 -g")

set(FALLBACK_GREETER_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/spawny )
set(PACKAGE_CONFIG_DIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/spawny)

set(PAM_SERVICE_GREETER \"spawny-greeter\")
set(PAM_SERVICE \"spawny\")

set(FALLBACK_GREETER \"${FALLBACK_GREETER_PATH}/sp-fallback-greeter\")
set(SERVER_SOCKET \"/tmp/spawny.socket\")

set(PACKAGE_DATA_DIR \"${CMAKE_INSTALL_FULL_DATADIR}/efm/\")
set(PACKAGE_LIB_DIR \"${CMAKE_INSTALL_FULL_LIBDIR}\")
set(PACKAGE_BIN_DIR \"${CMAKE_INSTALL_FULL_BINDIR}\")
set(PACKAGE_CONFIG \"${PACKAGE_CONFIG_DIR}/spawny.ini\")

add_subdirectory(src/libsp_client)
add_subdirectory(src/sp-fallback-greeter)
add_subdirectory(src/sp-greeter-start)
add_subdirectory(src/sp-daemon)
add_subdirectory(src/tests)
add_subdirectory(data)

configure_file(${CMAKE_SOURCE_DIR}/src/config/config.h.cmake ${CMAKE_SOURCE_DIR}/src/config/config.h)