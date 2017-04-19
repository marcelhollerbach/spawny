[Unit]
Description="A daemon which prompts for logins and starts a session after that"

[Service]
ExecStart=@CMAKE_INSTALL_FULL_BINDIR@/sp-daemon
