include_directories(
    ${PROJECT_SOURCE_DIR}/components/inih/
    ${PROJECT_SOURCE_DIR}/src/libsp_client/
    ${PROJECT_SOURCE_DIR}/src/config/
)

set(SPAWNY-DAEMON-TEST-SOURCE
    test.c
    test.h
    test_manager.c
    test_templatereg.c
    ../sp-daemon/manager.h
    ../sp-daemon/manager.c
    ../sp-daemon/spawnregistery.c
    ../sp-daemon/spawnregistery.h
    ../sp-daemon/sessionmgt.h
    ../sp-daemon/sessionmgt_logind.c
    ../sp-daemon/templatereg.h
    ../sp-daemon/templatereg.c
)

add_executable(test_spawny_daemon ${SPAWNY-DAEMON-TEST-SOURCE})
target_link_libraries (test_spawny_daemon
    check
    pam
    systemd
    protobuf-c
)

add_test(test_spawny_daemon ${CMAKE_CURRENT_BINARY_DIR}/test_spawny_daemon)