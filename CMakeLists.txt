cmake_minimum_required (VERSION 2.6)
project (Spawny)

include(GNUInstallDirs)
enable_testing()

add_definitions("-Wall -O0 -g")

add_subdirectory(src/greeter_start)
add_subdirectory(src/cli-greeter)
add_subdirectory(src/daemon)
add_subdirectory(src/client)
add_subdirectory(src/tests)
add_subdirectory(data)