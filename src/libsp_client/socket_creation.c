#include "Sp_Client.h"
#include "sp_client_private.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

const char*
sp_service_path_get(void)
{
    return SERVER_SOCKET;
}

void
sp_service_address_setup(struct sockaddr_un *in)
{
    memset(in, 0, sizeof(*in));
    in->sun_family = AF_UNIX;
    snprintf(in->sun_path, sizeof(in->sun_path), SERVER_SOCKET);
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
sp_service_connect(void) {
    int server_sock;
    struct sockaddr_un address;

    server_sock = sp_service_socket_create();

    sp_service_address_setup(&address);

    if (connect(server_sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
       perror("Failed to connect to service");
       close(server_sock);
       return -1;
    }

    return server_sock;
}