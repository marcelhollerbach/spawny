[Unit]
Description="A unit that prompts for a login to a session once the grahpical target is reached"

[Service]
ExecStart=@PACKAGE_BIN_DIR@/sp-greeter-start

[Install]
WantedBy=graphical.target
