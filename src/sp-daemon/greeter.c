#include "main.h"
#include <libgen.h>
#include <unistd.h>

typedef struct
{
    Spawn_Try *try;
    pid_t pid;
} Seat_Greeter_Run;

typedef struct {
    const char *seat; /* the seat where this is running in */
    int run_gen; /* the generation which is currently running */
    Seat_Greeter_Run run;
    int end; /* set if this is meant to be closed */
} Seat_Greeter;

static Seat_Greeter *greeters;
static int len = 0;


#define FALLBACK_RUN_GENERATION 5

static Seat_Greeter*
_greeter_add(const char *seat) {
   len++;

   greeters = realloc(greeters, len * sizeof(Seat_Greeter));
   memset(&greeters[len - 1], 0, sizeof(Seat_Greeter));
   greeters[len - 1].seat = strdup(seat);
   greeters[len - 1].run.pid = -1;

   return &greeters[len - 1];
}

static void
_greeter_run_reset(Seat_Greeter *greeter) {
    memset(&greeter->run, 0, sizeof(Seat_Greeter_Run));
    greeter->run.pid = -1;
}

static Seat_Greeter*
_greeter_search(const char *seat) {
    for (int i = 0; i < len; i++) {
        if (!strcmp(greeters[i].seat, seat))
            return &greeters[i];
    }

    return NULL;
}

static void
_greeter_del(const char *seat) {
    int mode = 0;

    for (int i = 0; i < len; i++) {
        switch(mode){
            case 0:
                if (!strcmp(greeters[i].seat, seat))
                  mode = 1;
            break;
            case 1:
                greeters[i - 1] = greeters[i];
            break;
        }
    }

    //realloc
    len --;
    greeters = realloc(greeters, len * sizeof(Seat_Greeter));
}


static void
_greeter_done(void *data, int status, pid_t pid) {
    Seat_Greeter *greeter;
    greeter = data;

    if (!greeter->end) {
        greeter->run_gen ++;

        printf("Greeter exited unexpected\n");

        _greeter_run_reset(greeter);
        greeter_activate(greeter->seat);

    } else {

        printf("Greeter shutted down!\n");
        _greeter_del(greeter->seat);
    }
}

static void
_greeter_start_done(void *data, Spawn_Service_End end) {
    Seat_Greeter *greeter;

    greeter = data;

    if (end.success == SPAWN_SERVICE_ERROR) {
        printf("Greeter died, reexecute! %s\n", end.message);

        _greeter_run_reset(greeter);
        greeter_activate(greeter->seat);
        return;
    } else {
        printf("Greeter started.\n");

        //keep track of the session
        greeter->run.pid = greeter->run.try->pid;
        greeter->run.try = NULL;

        spawnregistery_listen(greeter->run.pid, _greeter_done, data);
    }

    greeter = NULL;
}

//runs in a seperated process
static void
_greeter_job(void *data) {
    char *cmd, *cmdpath;
    Seat_Greeter *greeter;

    greeter = data;

    if (greeter->run_gen < FALLBACK_RUN_GENERATION)
        cmdpath = (char*)config->greeter.cmd;

    //we are save here with greader equals, basically we never reach this point with the normal activision
    if (greeter->run_gen >= FALLBACK_RUN_GENERATION || !cmdpath)
        cmdpath = FALLBACK_GREETER;

    cmd = basename(cmdpath);

    printf("Starting greeter app %s\n", cmdpath);
    execl(cmdpath, cmd, NULL);
    exit(1);
}

void
greeter_activate(const char *seat) {
    Seat_Greeter *greeter;
    char *session = NULL;

    greeter = _greeter_search(seat);
    if (!greeter) greeter = _greeter_add(seat);


    if (greeter->run.try) {
        session = sesison_get(greeter->run.try->pid);
    } else if (greeter->run.pid != -1) { /* pid 0 can NEVER be a greeter */
        session = sesison_get(greeter->run.pid);
    }

    if (session) {
        session_activate(session);
        free(session);
        return;
    }

    if (!(greeter->run_gen > FALLBACK_RUN_GENERATION))
      greeter->run.try = spawnservice_spawn(_greeter_start_done, greeter, _greeter_job, greeter,
                                        PAM_SERVICE_GREETER, config->greeter.start_user, NULL);
}

void
greeter_lockout(const char *seat) {
    Seat_Greeter *greeter;

    greeter = _greeter_search(seat);

    if (!greeter) {
        printf("Failed to lockout greeter for seat %s\n", seat);
        return;
    }

    greeter->end = 1;
}
