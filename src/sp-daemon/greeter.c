#include "main.h"

#include <string.h>
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

static Array *greeters;

ARRAY_API(Seat_Greeter)

void
greeter_init(void)
{
   greeters = array_Seat_Greeter_new();
}

void
greeter_shutdown(void)
{
   array_free(greeters);
   greeters = NULL;
}

#define FALLBACK_RUN_GENERATION 5

static Seat_Greeter*
_greeter_add(const char *seat) {
   Seat_Greeter *greeter;

   greeter = array_Seat_Greeter_add(greeters);

   memset(greeter, 0, sizeof(Seat_Greeter));
   greeter->seat = strdup(seat);
   greeter->run.pid = -1;
   return greeter;
}

static void
_greeter_run_reset(Seat_Greeter *greeter) {
    memset(&greeter->run, 0, sizeof(Seat_Greeter_Run));
    greeter->run.pid = -1;
}

static Seat_Greeter*
_greeter_search(const char *seat) {
    for (int i = 0; i < array_len_get(greeters); i++) {
        Seat_Greeter *greeter = array_Seat_Greeter_get(greeters, i);
        if (!strcmp(greeter->seat, seat))
            return greeter;
    }

    return NULL;
}

static void
_greeter_del(const char *seat) {
    for (int i = 0; i < array_len_get(greeters); i++) {
        Seat_Greeter *greeter = array_Seat_Greeter_get(greeters, i);
        if (!strcmp(greeter->seat, seat))
          {
             array_Seat_Greeter_del(greeters, i);
             return;
          }
    }
}


static void
_greeter_done(void *data, int status, pid_t pid) {
    Seat_Greeter *greeter;
    greeter = data;

    if (!greeter->end) {
        greeter->run_gen ++;

        ERR("Greeter exited unexpected");

        _greeter_run_reset(greeter);
        greeter_activate(greeter->seat);

    } else {

        INF("Greeter shutted down!");
        _greeter_del(greeter->seat);
    }
}

static void
_greeter_start_done(void *data, Spawn_Service_End end) {
    Seat_Greeter *greeter;

    greeter = data;

    if (end.success == SPAWN_SERVICE_ERROR) {
        greeter->run_gen ++;

        ERR("Greeter died reason: (%s), reexecute!", end.message);

        _greeter_run_reset(greeter);
        greeter_activate(greeter->seat);
        return;
    } else {
        INF("Greeter started.");

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

    printf("Starting greeter app %s", cmdpath);
    execl(cmdpath, cmd, NULL);
    exit(1);
}

void
greeter_activate(const char *seat) {
    Seat_Greeter *greeter;
    Xdg_Settings settings;
    char *session = NULL;

    greeter = _greeter_search(seat);
    if (!greeter) greeter = _greeter_add(seat);

    memset(&settings, 0, sizeof(Xdg_Settings));
    settings.session_seat = greeter->seat;

    if (greeter->run.try) {
        session = session_get(greeter->run.try->pid);
    } else if (greeter->run.pid != -1) { /* pid 0 can NEVER be a greeter */
        session = session_get(greeter->run.pid);
    }

    if (session) {
        session_activate(session);
        free(session);
        return;
    }

    if (!(greeter->run_gen > FALLBACK_RUN_GENERATION))
      greeter->run.try = spawnservice_spawn(_greeter_start_done, greeter, _greeter_job, greeter,
                                        PAM_SERVICE_GREETER, config->greeter.start_user, NULL, &settings);
    else
      ERR("Starting the greeter %d times did not bring it up, giving up. :(", greeter->run_gen);
}

void
greeter_lockout(const char *seat) {
    Seat_Greeter *greeter;

    greeter = _greeter_search(seat);

    if (!greeter) {
        ERR("Failed to lockout greeter for seat %s", seat);
        return;
    }

    greeter->end = 1;
}

static pid_t
seat_greeter_pid_get(Seat_Greeter *seat) {
    if (seat->run.try) return seat->run.try->pid;
    return seat->run.pid;
}

bool
greeter_exists_sid(pid_t sid) {
    for (int i = 0; i < array_len_get(greeters); ++i) {
        Seat_Greeter *greeter = array_Seat_Greeter_get(greeters, i);
        pid_t greeter_sid = getsid(seat_greeter_pid_get(greeter));
        if (greeter_sid == -1) {
            ERR("Failed to fetch sid from %d", greeter->run.pid);
            continue;
        }
        if (greeter_sid == sid) return true;
    }

    return false;
}
