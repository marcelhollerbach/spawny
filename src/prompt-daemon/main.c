#include "main.h"
#include <unistd.h>
#include <libgen.h>
#include <signal.h>

#define GREETER_SERVICE "lightdm-greeter"

typedef enum {
    STATE_INIT = 0,
    STATE_GREETER_STARTED,
} Run_State;

static Run_State state;

static void
sigfunc(int sig) {
    server_shutdown();
}

static void
_spawn_msg(void *data, int success) {
    switch(state){
        case STATE_INIT:
            if (!success) {
                printf("Panic! The worlrd is on fire!\n");
                manager_stop();
                return;
            } else {
                printf("Greeter started.\n");
                state = STATE_GREETER_STARTED;
            }
        break;
        case STATE_GREETER_STARTED:
            if (!success) {
                server_spawnservice_feedback(0, "char *message");
            } else {
                server_spawnservice_feedback(1, "char *message");
                printf("User Session alive.\n");
                manager_stop();
            }
        break;
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

    signal(SIGINT, sigfunc);
    signal(SIGSEGV, sigfunc);

    config_init();
    manager_init();

    tty_template_init();

    spawnservice_init(_spawn_msg, NULL);

    if (!server_init()) {
        return -1;
    }
    printf("Init done\n");
    if (!spawnservice_spawn(_greeter_job, GREETER_SERVICE, config->greeter.start_user, NULL, NULL)) {
        printf("Starting greeter failed");
        return -1;
    }

    manager_run();

    server_shutdown();
    config_shutdown();
}