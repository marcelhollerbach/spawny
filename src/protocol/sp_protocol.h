#ifndef SP_PROTOCOL_H
#define SP_PROTOCOL_H

#include "greeter.pb-c.h"
#include "server.pb-c.h"
#include "config.h"

#include <sys/un.h>

/*
 *
 */
const char* sp_service_path_get(void);

/*
 *
 */
int sp_service_socket_create(void);

/*
 *
 */
void sp_service_address_setup(struct sockaddr_un *in);

/*
 *
 */
int sp_service_connect(void);

#endif