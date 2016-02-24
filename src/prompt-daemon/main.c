#include "main.h"
#include <unistd.h>
#include <libgen.h>
#include <signal.h>

#define GREETER_SERVICE "lightdm-greeter"

static void
_greeter_done(void *data, Spawn_Service_End end) {
    if (end.success == SPAWN_SERVICE_ERROR) {
        printf("Panic! The worlrd is on fire!\n");
        manager_stop();
        return;
    } else {
        printf("Greeter started.\n");
    }
}

//runs in a seperated process
static void
_greeter_job(void *data) {
    char *cmd, *cmdpath = (char*)config->greeter.cmd;

    cmd = basename(cmdpath);

    printf("Starting greeter app %s\n", cmdpath);
    execl(cmdpath, cmd, NULL);
    exit(1);
}

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

    tty_template_init();
    x11_template_init();
    wl_template_init();

    if (!server_init()) {
        return -1;
    }
    printf("Init done\n");

    if (!spawnservice_spawn(_greeter_done, NULL, _greeter_job, NULL, GREETER_SERVICE, config->greeter.start_user, NULL)) {
        printf("Starting greeter failed");
        return -1;
    }

    manager_run();

    server_shutdown();
    config_shutdown();
}