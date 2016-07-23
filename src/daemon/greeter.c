#include "main.h"
#include <libgen.h>
#include <unistd.h>

#define GREETER_SERVICE "lightdm-greeter"
#define FALLBACK_GREETER PACKAGE_LIB_DIR"/spawny/sp-fallback-greeter";


static int fallback = 0;

static Spawn_Try *greeter;
static char *session;
static pid_t pid;

static void
_greeter_done(void *data, int status, pid_t pid) {
    //TODO check if this greeter session is at the end.
    pid = -1;
    free(session);
    session = NULL;
}

static void
_greeter_start_done(void *data, Spawn_Service_End end) {
    if (end.success == SPAWN_SERVICE_ERROR) {
        printf("Panic! The worlrd is on fire!\n");
        //TODO fallback to commandline, or dont do anything
        return;
    } else {
        printf("Greeter started.\n");
        //keep track of the session
        session = strdup(greeter->session);
        pid = greeter->pid;

        spawnregistery_listen(pid, _greeter_done, NULL);
    }

    greeter = NULL;
}

//runs in a seperated process
static void
_greeter_job(void *data) {
    char *cmd, *cmdpath;

    if (!fallback)
        cmdpath = (char*)config->greeter.cmd;

    if (fallback || !cmdpath)
        cmdpath = FALLBACK_GREETER;

    cmd = basename(cmdpath);

    printf("Starting greeter app %s\n", cmdpath);
    execl(cmdpath, cmd, NULL);
    exit(1);
}

void
activate_greeter(void)
{
    if (greeter) {
        session_activate(greeter->session);
        return;
    }

    if (session && pid) {
        session_activate(session);
        return;
    }

    greeter = spawnservice_spawn(_greeter_start_done, NULL, _greeter_job, NULL,
                                 GREETER_SERVICE, config->greeter.start_user, NULL);
}