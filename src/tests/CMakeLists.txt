set(SPAWNY-DAEMON-TEST-SOURCE
    test.c
    test.h
    test_manager.c
    test_templatereg.c
    ../daemon/manager.h
    ../daemon/manager.c
    ../daemon/spawnservice.c
    ../daemon/spawnservice.h
    ../daemon/spawnregistery.c
    ../daemon/spawnregistery.h
    ../daemon/sessionmgt.h
    ../daemon/sessionmgt_logind.c
    ../daemon/templatereg.h
    ../daemon/templatereg.c
)

add_executable(test_spawny_daemon ${SPAWNY-DAEMON-TEST-SOURCE})
target_link_libraries (test_spawny_daemon
    check
    pam
    systemd
    protobuf-c
    spawny_protocol
)

add_test(test_spawny_daemon ${CMAKE_CURRENT_BINARY_DIR}/test_spawny_daemon)