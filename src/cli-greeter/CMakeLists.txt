set(SPAWNY-CLC-SOURCE
    main.c
)

add_executable(spawny-clc ${SPAWNY-CLC-SOURCE})

target_link_libraries (spawny-clc
    protobuf-c
    spawny_protocol
    spawny_client
)

install(TARGETS spawny-clc DESTINATION ${CMAKE_INSTALL_LIBDIR}/spawny/)