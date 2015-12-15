set(SPAWNY-CLC-SOURCE
    main.c
    main.h
    client.c
    ${PROJECT_SOURCE_DIR}/src/protocol/protocol.h
)

add_executable(spawny-clc ${SPAWNY-CLC-SOURCE})

target_link_libraries (spawny-clc
    protobuf-c
    spawny_protocol
)

install(TARGETS spawny-clc DESTINATION ${CMAKE_INSTALL_LIBDIR}/spawny/)