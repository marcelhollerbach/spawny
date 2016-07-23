set(SPAWNY-CLC-SOURCE
    main.c
)

add_executable(sp-fallback-greeter ${SPAWNY-CLC-SOURCE})

target_link_libraries (sp-fallback-greeter
    protobuf-c
    spawny_protocol
    spawny_client
)

install(TARGETS sp-fallback-greeter DESTINATION ${CMAKE_INSTALL_LIBDIR}/spawny/)