# Protocol

A greeter is a piece of software that connects to the server socket of the daemon.

# Greeter to daemon

A greeter can send requests to the daemon.

## Hello
After a 'Hello' message the daemon replies with a Data message. Further requests can be sent

## Activate Session
The message must contain the id of the Session object that should get acitvated.
The daemon will answer with a request feedback object.

## Login
The message must contain a valid user name, password, and the id of the template that should get instanciated.
The daemon will answer with a request feedback object.

## Greeter start
The message results in the daemon trying to start a new session where the greeter is brought up.
After this request the daemon will close the communication connection. No further requests will be allowed.

# Daemon to Greeter

The daemon can sent messages to the greeter. Messages are only sent back as reply to a previous command.

## Data
This brings back a data obejct that contains all objects of sessions users and templates.

## Login Feedback
If a request to login is sent this object indicates if the login was successfull or not. If the request was successfull the daemon will close the connection
