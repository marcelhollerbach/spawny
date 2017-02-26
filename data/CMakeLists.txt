INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny.ini DESTINATION ${PACKAGE_CONFIG_DIR})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny DESTINATION /etc/pam.d/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/spawny-greeter DESTINATION /etc/pam.d/)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sp-daemon.service DESTINATION /etc/systemd/system)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sp-greeter-start.service DESTINATION /etc/systemd/system)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sp-daemon.socket DESTINATION /etc/systemd/system)
