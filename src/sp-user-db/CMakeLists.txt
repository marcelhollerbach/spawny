include_directories(
    ${PROJECT_SOURCE_DIR}/src/sp-daemon/
    ${INIH_INCLUDE_DIRS}
)

set(SP-USER_DB
    user_db.c
    user_db.c
    ${PROJECT_SOURCE_DIR}/src/sp-daemon/utils.c
    ${PROJECT_SOURCE_DIR}/src/sp-daemon/utils.h
    #${PROJECT_SOURCE_DIR}/components/inih/ini.c
)

add_library(SpUserDb STATIC ${SP-USER_DB})

target_link_libraries (SpUserDb
    inih
)
