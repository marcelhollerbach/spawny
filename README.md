# spawny

Spawny presents a daemon which can prompt for logins on ttys.
It does that by presenting 3 binaries.

##sp-daemon

The daemon opens a socket and listens there for requests.
If someone requests a new session or the displaying of a greeter, the following steps are done:
* A new process is forked.
* The new process calls setsid() so its a new session
* A new tty console is opened and added, the process is attached as leader
* The user is logged in using pam
* Waiting for its new session to be activated
* Starts up the greeter or the session-template which was applied

The daemon also loads its session-templates form /usr/xsessions /user/wayland-session and offers a tty template which just starts the users bash

A different greeter from the sp-fallback-greeter can be configured in the config file.

##sp-falback-greeter

The repository also brings its own greeter, it uses the command line and no graphics.
It is used as fallback-greeter if the configured one fails.

The fallback greeter looks like that:

![Fallback greeter](https://cloud.githubusercontent.com/assets/1415748/17888029/6235d18a-6929-11e6-9f77-87d934d70be0.png)

##sp-greeter-start

Requests the daemon to start a greeter on the same seat where this process is in.
