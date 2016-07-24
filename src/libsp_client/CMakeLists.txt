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
       OUTPUT ${PROJECT_SOURCE_DIR}/src/libsp_client/${part}.pb-c.c
       OUTPUT ${PROJECT_SOURCE_DIR}/src/libsp_client/${part}.pb-c.h
       COMMAND cd ${PROJECT_SOURCE_DIR}/src/libsp_client && protoc-c ${part}.proto --c_out=${PROJECT_SOURCE_DIR}/src/libsp_client
       DEPENDS ${part}.proto
    )
endforeach()

set(SPAWNY_CLIENT_LIB
  client.c
  socket_creation.c
  server.pb-c.c
  server.pb-c.h
  greeter.pb-c.c
  greeter.pb-c.h
  Sp_Client.h
)

add_library(sp_client SHARED
            ${SPAWNY_CLIENT_LIB})

target_link_libraries (sp_client
    protobuf-c
)

install(TARGETS sp_client DESTINATION ${CMAKE_INSTALL_LIBDIR})