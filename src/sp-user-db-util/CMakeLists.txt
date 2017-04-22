include_directories(
    ${PROJECT_SOURCE_DIR}/src/sp-user-db/
)

set(SPAWNY-USER_DB
    user_db_util.c
)

add_executable(sp-user-db-util ${SPAWNY-USER_DB})

target_link_libraries (sp-user-db-util
    SpUserDb
)
install(TARGETS sp-user-db-util DESTINATION ${CMAKE_INSTALL_BINDIR})
