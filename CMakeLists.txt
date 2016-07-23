cmake_minimum_required (VERSION 2.6)
project (Spawny)

include(GNUInstallDirs)
enable_testing()

add_definitions("-Wall -O0 -g")

add_subdirectory(src/libsp_client)
add_subdirectory(src/sp-fallback-greeter)
add_subdirectory(src/sp-greeter-start)
add_subdirectory(src/sp-daemon)
add_subdirectory(src/tests)
add_subdirectory(data)