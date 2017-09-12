install_data(['spawny', 'spawny-greeter'],
  install_dir : '/etc/pam.d/'
)

install_data(['spawny.ini.sample'],
  install_dir : config_h.get('PACKAGE_DATA_DIR', '/etc/')
)

systemd_files = ['sp-daemon.service.cmake', 'sp-daemon.socket.cmake', 'sp-greeter-start.service.cmake']

foreach file : systemd_files
  configure_file(input : file, output : '@BASENAME@',
    install_dir : '/etc/systemd/system',
    configuration : config_h
  )
endforeach

