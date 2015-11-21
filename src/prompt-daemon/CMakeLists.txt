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
)

add_executable(spawny-daemon ${SPAWNY-DAEMON-SOURCE})

target_link_libraries (spawny-daemon
    pam
    systemd
    spawny_protocol
)

install(TARGETS spawny-daemon DESTINATION bin)