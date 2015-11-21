cmake_minimum_required (VERSION 2.6)
project (Spawny)

enable_testing()

add_definitions("-Wall -O0 -g")

add_subdirectory(src/protocol)
add_subdirectory(src/prompt-daemon)
add_subdirectory(src/cli-greeter)
add_subdirectory(src/tests)
add_subdirectory(data)