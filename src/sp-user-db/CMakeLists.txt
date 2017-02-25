include_directories(
    ${PROJECT_SOURCE_DIR}/components/inih/
)

set(SP-USER_DB
    user_db.c
    user_db.c
    ${PROJECT_SOURCE_DIR}/components/inih/ini.c
)

add_library(SpUserDb STATIC ${SP-USER_DB})
