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
       OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${part}.pb-c.c
       OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${part}.pb-c.h
       COMMAND protoc-c ${part}.proto --c_out=${CMAKE_CURRENT_BINARY_DIR}
       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/sp-protocol
       DEPENDS ${part}.proto
    )
endforeach()

set(SP_PROTOCOL_LIB
  ${CMAKE_CURRENT_BINARY_DIR}/server.pb-c.c
  ${CMAKE_CURRENT_BINARY_DIR}/server.pb-c.h
  ${CMAKE_CURRENT_BINARY_DIR}/greeter.pb-c.c
  ${CMAKE_CURRENT_BINARY_DIR}/greeter.pb-c.h
  socket_creation.c
  sp_protocol.h
)

add_library(SpProtocol STATIC
            ${SP_PROTOCOL_LIB})

target_link_libraries (SpProtocol
    protobuf-c
)
