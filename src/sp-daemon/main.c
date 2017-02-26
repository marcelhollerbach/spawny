#include "main.h"
#include <unistd.h>


static void
init_check(void) {
    const char *session;

    if (getuid() != 0) {
        printf("You can only run this as root.\n");
        exit(-1);
    }

    session = current_session_get();

    if (session != NULL) {
        printf("You must start this out of a session. This is in session %s\n", session);
        exit(-1);
    }
}

int
main(int argc, char **argv)
{
    init_check();
    config_init();
    manager_init();
    spawnservice_init();
    spawnregistery_init();

    tty_template_init();
    x11_template_init();
    wl_template_init();

    if (!server_init()) {
        return -1;
    }

    printf("Init done\n");
    manager_run();

    server_shutdown();
    config_shutdown();
}
