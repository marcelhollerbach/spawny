set(PROTOCOL_FILES
    session_spawn
)

#compile protobuf files

foreach(part ${PROTOCOL_FILES})
    add_custom_command(
       OUTPUT ${PROJECT_SOURCE_DIR}/src/daemon/${part}.pb-c.c
       OUTPUT ${PROJECT_SOURCE_DIR}/src/daemon/${part}.pb-c.h
       COMMAND cd ${PROJECT_SOURCE_DIR}/src/daemon && protoc-c ${part}.proto --c_out=${PROJECT_SOURCE_DIR}/src/daemon
       DEPENDS ${part}.proto
    )
endforeach()

set(SPAWNY-DAEMON-SOURCE
    main.c
    config.c
    config.h
    manager.h
    manager.c
    spawnservice.c
    spawnservice.h
    spawnregistery.c
    spawnregistery.h
    greeter.c
    greeter.h
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
    desktop_common.c
    desktop_common.h
    session_spawn.pb-c.c
    session_spawn.pb-c.h
    ${PROJECT_SOURCE_DIR}/components/inih/ini.h
    ${PROJECT_SOURCE_DIR}/components/inih/ini.c
)

include_directories(
    ${PROJECT_SOURCE_DIR}/components/inih/
)

add_executable(sp-daemon ${SPAWNY-DAEMON-SOURCE})

target_link_libraries (sp-daemon
    protobuf-c
    pam
    systemd
    spawny_client
)

set_property(TARGET sp-daemon PROPERTY COMPILE_DEFINITIONS
    PACKAGE_LIB_DIR="${CMAKE_INSTALL_FULL_LIBDIR}"
    PACKAGE_ETC_DIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

install(TARGETS sp-daemon DESTINATION ${CMAKE_INSTALL_BINDIR})