# spawny

[![CircleCI](https://circleci.com/gh/marcelhollerbach/spawny.svg?style=svg)](https://circleci.com/gh/marcelhollerbach/spawny)

Spawny presents a daemon which can prompt for logins on ttys.
It is splitted up into multiple subsystems

## Installation
The repository is using a external .ini parser that can get pulled in by running:

`git update --init --recursive`

To build the software you can just create a bin directory like build,

`mkdir build`

After that you can build and install the software by doing:

`cd build`

`cmake ..`

`make all`

`sudo make install`

## sp-daemon

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

## sp-fallback-greeter

This is a greeter that just uses the command line interface. It is used as fallback for the case a configured greeter crashes 3 times, or cannot be started / found.

The fallback greeter looks like that:

![Fallback greeter](https://cloud.githubusercontent.com/assets/1415748/17888029/6235d18a-6929-11e6-9f77-87d934d70be0.png)

## sp-greeter-start

Requests the daemon to start a greeter on the same seat where this process is in.

## libsp_client

A little shared object library that abstracts the communication with the deamon. It can be used by other greeters, it is also used by sp-greeter-start and sp-fallback-greeter.

## sp-protocol

The whole repository needs a few protobuf files to work correctly. This little static library brings in the generated files from the protobuf files and a few other functions that are used by the sp-daemon and the libsp_client.

## sp-user-db(-utils)

There is one problem with the /etc/passwd file on linux systems. there are very static and you cannot add any additional data to them. For something like spawny and a graphical greeter it could be very usefull to have a user configured icon or prefered template to use. Those are the packages that are implementing that

