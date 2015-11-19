set(PROTOCOL_FILES
    session_spawn
    greeter
    server
)

#compile protobuf files

foreach(part ${PROTOCOL_FILES})
    add_custom_command(
       OUTPUT ${PROJECT_SOURCE_DIR}/src/protocol/${part}.pb-c.c
       OUTPUT ${PROJECT_SOURCE_DIR}/src/protocol/${part}.pb-c.h
       COMMAND cd ${PROJECT_SOURCE_DIR}/src/protocol && protoc-c ${part}.proto --c_out=${PROJECT_SOURCE_DIR}/src/protocol
       DEPENDS ${part}.proto
    )
endforeach()

set(SPAWNY_PROTOCOL
    session_spawn.pb-c.c
    session_spawn.pb-c.h
    server.pb-c.c
    server.pb-c.h
    greeter.pb-c.c
    greeter.pb-c.h
)

add_library(spawny_protocol SHARED
            ${SPAWNY_PROTOCOL})
target_link_libraries (spawny_protocol
    protobuf-c
)
install(TARGETS spawny_protocol DESTINATION lib)