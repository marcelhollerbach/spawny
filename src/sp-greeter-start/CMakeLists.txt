set(SPAWNY-GREETER-START
    main.c
)

include_directories(
    ../client/
)

add_executable(sp-greeter-start ${SPAWNY-GREETER-START})

target_link_libraries (sp-greeter-start
    protobuf-c
    spawny_protocol
    spawny_client
)