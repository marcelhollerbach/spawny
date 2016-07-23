#include "main.h"
#include <libgen.h>
#include <unistd.h>

#define GREETER_SERVICE "lightdm-greeter"

static Spawn_Try *greeter;

static void
_greeter_done(void *data, Spawn_Service_End end) {
    if (end.success == SPAWN_SERVICE_ERROR) {
        printf("Panic! The worlrd is on fire!\n");
        manager_stop();
        return;
    } else {
        printf("Greeter started.\n");
    }

    greeter = NULL;
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

void
activate_greeter(void)
{
   if (!greeter)
     {
        greeter =
           spawnservice_spawn(_greeter_done, NULL, _greeter_job, NULL,
           GREETER_SERVICE, config->greeter.start_user, NULL);
     }
   else
     {
        session_activate(greeter->session);
     }
}