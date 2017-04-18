#include "main.h"
#include <systemd/sd-daemon.h>

int prefetch_server_socket(void)
{
    int n =  sd_listen_fds(0);
    if (n == 0) /* silent return, we are just not started from systemd */ return 0;

    if (n != 1) {
        ERR("Error, systemd passed to many fd´s!");
        return 0;
    } else {
        return SD_LISTEN_FDS_START + 0;
    }
}
