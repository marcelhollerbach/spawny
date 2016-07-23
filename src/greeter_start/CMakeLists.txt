set(SPAWNY-GREETER-START
    main.c
)

include_directories(
    ../client/
)

add_executable(spawny-greeter-start ${SPAWNY-GREETER-START})

target_link_libraries (spawny-greeter-start
    protobuf-c
    spawny_protocol
    spawny_client
)