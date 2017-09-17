install_data(['spawny', 'spawny-greeter'],
  install_dir : '/etc/pam.d/'
)

systemd_files = ['sp-daemon.service.cmake', 'sp-daemon.socket.cmake', 'sp-greeter-start.service.cmake']

in_config = configuration_data()
in_config.set('SP_SERVER_SOCKET', server_socket)
in_config.set('PACKAGE_BIN_DIR', bin_dir)

foreach file : systemd_files
  configure_file(input : file, output : '@BASENAME@',
    install_dir : '/etc/systemd/system',
    configuration : in_config
  )
endforeach

