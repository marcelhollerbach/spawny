configure_file(${CMAKE_SOURCE_DIR}/pc/sp-client.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/sp-client.pc @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sp-client.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig/)
