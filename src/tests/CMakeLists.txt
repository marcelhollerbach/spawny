set(SPAWNY-DAEMON-TEST-SOURCE
    test.c
    test.h
    test_manager.c
    test_templatereg.c
    ../prompt-daemon/manager.h
    ../prompt-daemon/manager.c
    ../prompt-daemon/spawnservice.c
    ../prompt-daemon/spawnservice.h
    ../prompt-daemon/sessionmgt.h
    ../prompt-daemon/sessionmgt_logind.c
    ../prompt-daemon/templatereg.h
    ../prompt-daemon/templatereg.c
    ${PROJECT_SOURCE_DIR}/src/protocol/protocol.h
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