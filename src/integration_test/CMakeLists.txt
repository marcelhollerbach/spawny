include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/config/
  ${CMAKE_SOURCE_DIR}/src/libsp_client/
)


set(chroot_dir ${CMAKE_BINARY_DIR}/chroot)

configure_file(binary_locations.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/binary_locations.h)

add_library(preload_systemd SHARED
            preload_systemd.c)

add_executable(suite
              test.h
              test_suite.c
              test_protocol.c
              test_greeter_start.c
              util.c
)

target_link_libraries (suite
   ${Check_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   SpClient
    m
)

add_custom_target(prepare-integration-test
  ${CMAKE_CURRENT_SOURCE_DIR}/prepare.sh ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS preload_systemd
)

add_custom_target(integration-test
  ${CMAKE_CURRENT_BINARY_DIR}/suite
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS suite
)
