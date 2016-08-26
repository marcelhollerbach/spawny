include(CMakePackageConfigHelpers)
include_directories(
    ${PROJECT_SOURCE_DIR}/src/config/
)

set(SPAWNY_CLIENT_LIB
  client.c
  Sp_Client.h
)

add_library(SpClient SHARED
            ${SPAWNY_CLIENT_LIB})

target_link_libraries (SpClient
    PRIVATE SpProtocol
    protobuf-c
)


install(TARGETS SpClient EXPORT SpClientTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES Sp_Client.h server.pb-c.h greeter.pb-c.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(ConfigProjectLocation ${PROJECT_BINARY_DIR}/libsp_client)
set(ConfigPackageLocation lib/cmake/SpClient)
set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(CONF_INSTALL_POSITION "${CMAKE_INSTALL_PREFIX}/${ConfigPackageLocation}")

#create version file

write_basic_package_version_file(
  "${ConfigProjectLocation}/SpClientVersion.cmake"
  VERSION 0.1
  COMPATIBILITY AnyNewerVersion
)

#export targets

export(EXPORT SpClientTargets
  FILE "${ConfigProjectLocation}/SpClientTargets.cmake"
)

configure_file(${CMAKE_SOURCE_DIR}/cmake/SpClientConfig.cmake.in
  "${ConfigProjectLocation}/SpClientConfig.cmake"
)

install(EXPORT SpClientTargets
  FILE
    SpClientTargets.cmake
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    "${ConfigProjectLocation}/SpClientConfig.cmake"
    "${ConfigProjectLocation}/SpClientVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
  )