#include "sp_protocol.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

const char*
sp_service_path_get(bool debug)
{
    if (debug)
      return SERVER_DEBUG_SOCKET;
    else
      return SERVER_SOCKET;
}

void
sp_service_address_setup(bool debug, struct sockaddr_un *in)
{
    const char *path;

    memset(in, 0, sizeof(*in));
    in->sun_family = AF_UNIX;

    path = sp_service_path_get(debug);

    snprintf(in->sun_path, sizeof(in->sun_path), "%s", path);
}

int
sp_service_socket_create(void) {
    int server_sock;

    server_sock = socket(PF_UNIX, SOCK_STREAM, 0);

    if (server_sock < 0) {
        perror("Failed to create server socket");
        return -1;
    }

    return server_sock;
}

int
sp_service_connect(bool debug) {
    int server_sock;
    struct sockaddr_un address;

    server_sock = sp_service_socket_create();

    if (server_sock < 0) {
        return -1;
    }

    sp_service_address_setup(debug, &address);

    if (connect(server_sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
       perror("Failed to connect to service");
       close(server_sock);
       return -1;
    }

    return server_sock;
}
