ExternalProject_Add(inih_fs
  GIT_REPOSITORY https://github.com/benhoyt/inih.git
  GIT_TAG r40
  PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists-inih.txt ${CMAKE_CURRENT_BINARY_DIR}/inih_fs-prefix/src/inih_fs/CMakeLists.txt
  INSTALL_COMMAND ""
)

add_library(inih STATIC IMPORTED GLOBAL)

set(BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/inih_fs-prefix/src/inih_fs")

SET(INIH_INCLUDE_DIRS  "${BASE_DIR}/" CACHE PATH "Path where to find inih include files")
SET(INIH_INSTALL_DIRS  "${BASE_DIR}-build/libinih.a")


set_property(TARGET inih
  PROPERTY
    IMPORTED_LOCATION "${INIH_INSTALL_DIRS}")

add_dependencies(inih inih_fs)
