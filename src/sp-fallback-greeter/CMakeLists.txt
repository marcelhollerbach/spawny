include_directories(
    ${PROJECT_SOURCE_DIR}/src/libsp_client/
)

set(SPAWNY-CLC-SOURCE
    main.c
)

add_executable(sp-fallback-greeter ${SPAWNY-CLC-SOURCE})

target_link_libraries (sp-fallback-greeter
    spawny_client
)

install(TARGETS sp-fallback-greeter DESTINATION ${FALLBACK_GREETER_PATH})