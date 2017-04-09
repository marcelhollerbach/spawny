#ifndef SP_CLIENT_PRIVATE_H
#define SP_CLIENT_PRIVATE_H

#include "greeter.pb-c.h"
#include "server.pb-c.h"
#include "config.h"

//lets steal the log.c and log.h from the sp-daemon
#include "../sp-daemon/log.h"

#include <sys/un.h>

const char* sp_service_path_get(void);
int sp_service_socket_create(void);
void sp_service_address_setup(struct sockaddr_un *in);
int sp_service_connect(void);

#endif
