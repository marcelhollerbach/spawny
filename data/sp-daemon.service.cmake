[Unit]
Description="A daemon which prompts for logins and starts a session after that"

[Service]
ExecStart=@PACKAGE_BIN_DIR_P@/sp-daemon
