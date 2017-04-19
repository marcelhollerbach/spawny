[Unit]
Description="A unit that prompts for a login to a session once the grahpical target is reached"

[Service]
ExecStart=@CMAKE_INSTALL_FULL_BINDIR@/sp-greeter-start

[Install]
WantedBy=graphical.target
