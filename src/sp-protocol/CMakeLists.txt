include_directories(
    ${PROJECT_SOURCE_DIR}/src/config/
)

set(PROTOCOL_FILES
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

set(SP_PROTOCOL_LIB
  server.pb-c.c
  server.pb-c.h
  greeter.pb-c.c
  greeter.pb-c.h
  socket_creation.c
  sp_protocol.h
)

add_library(SpProtocol STATIC
            ${SP_PROTOCOL_LIB})

target_link_libraries (SpProtocol
    protobuf-c
)