include_directories(
    ${PROJECT_SOURCE_DIR}/src/libsp_client/
)

set(SPAWNY-GREETER-START
    main.c
)

add_executable(sp-greeter-start ${SPAWNY-GREETER-START})

target_link_libraries (sp-greeter-start
    SpClient
)

install(TARGETS sp-greeter-start DESTINATION ${CMAKE_INSTALL_BINDIR})