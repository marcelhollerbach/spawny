INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny.ini.sample DESTINATION ${PACKAGE_CONFIG_DIR})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny DESTINATION /etc/pam.d/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny-greeter DESTINATION /etc/pam.d/)

configure_file(${CMAKE_SOURCE_DIR}/data/sp-daemon.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/sp-daemon.service)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sp-daemon.service DESTINATION /etc/systemd/system)

configure_file(${CMAKE_SOURCE_DIR}/data/sp-greeter-start.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/sp-greeter-start.service)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sp-greeter-start.service DESTINATION /etc/systemd/system)

configure_file(${CMAKE_SOURCE_DIR}/data/sp-daemon.socket.cmake ${CMAKE_CURRENT_BINARY_DIR}/sp-daemon.socket)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sp-daemon.socket DESTINATION /etc/systemd/system)
