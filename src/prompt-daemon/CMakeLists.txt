set(SPAWNY-DAEMON-SOURCE
    main.c
    config.c
    config.h
    manager.h
    manager.c
    spawnservice.c
    spawnservice.h
    sessionmgt.h
    sessionmgt_logind.c
    server.c
    server.h
    templatereg.h
    templatereg.c
    templates.h
    tty_temp.c
    wl_temp.c
    x11_temp.c
    ${PROJECT_SOURCE_DIR}/src/protocol/protocol.h
    ${PROJECT_SOURCE_DIR}/components/inih/ini.h
    ${PROJECT_SOURCE_DIR}/components/inih/ini.c
)

include_directories(
    ${PROJECT_SOURCE_DIR}/components/inih/
)

add_executable(spawny-daemon ${SPAWNY-DAEMON-SOURCE})

target_link_libraries (spawny-daemon
    pam
    systemd
    spawny_protocol
)

set_property(TARGET spawny-daemon PROPERTY COMPILE_DEFINITIONS
    PACKAGE_LIB_DIR="${CMAKE_INSTALL_FULL_LIBDIR}"
    PACKAGE_ETC_DIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

install(TARGETS spawny-daemon DESTINATION ${CMAKE_INSTALL_BINDIR})