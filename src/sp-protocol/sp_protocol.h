#ifndef SP_PROTOCOL_H
#define SP_PROTOCOL_H

#include "greeter.pb-c.h"
#include "server.pb-c.h"
#include "config.h"

#include <stdbool.h>
#include <sys/un.h>

/*
 * @return the path where the service socket is running on
 */
const char* sp_service_path_get(bool debug);

/*
 * Creates a socket with the desired options and parameters.
 *
 * @return result of socket(...), return < 0 on failure,
           error messages will be printed
 */
int sp_service_socket_create(void);

/*
 * Setup the given sockaddr_un struct to the desired options and parameters.
 *
 * @param the struct to fill
 */
void sp_service_address_setup(bool debug, struct sockaddr_un *in);

/*
 * Connect to the sp service.
 *
 * @return a fd to communicate with, < 0 on failure, errors are printed
 */
int sp_service_connect(bool debug);

#endif
