#include "main.h"
#include <libgen.h>
#include <unistd.h>

typedef enum {
    RUNNING = 0,
    FALLBACK = 1
} Running;

typedef struct {
    const char *seat;
    Spawn_Try *try;
    char *session;
    pid_t pid;
    Running mode;
    int end;
    int run_gen;
} Seat_Greeter;

static Seat_Greeter *greeters;
static int len = 0;

static Seat_Greeter*
_greeter_add(const char *seat) {
   len++;

   greeters = realloc(greeters, len * sizeof(Seat_Greeter));
   memset(&greeters[len - 1], 0, sizeof(Seat_Greeter));
   greeters[len - 1].seat = strdup(seat);

   return &greeters[len - 1];
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
        if (greeter->mode == FALLBACK)
            greeter->run_gen ++;
        greeter->mode = RUNNING;
        greeter_activate(greeter->seat);
    } else {
        _greeter_del(greeter->seat);
    }
}

static void
_greeter_start_done(void *data, Spawn_Service_End end) {
    Seat_Greeter *greeter;

    greeter = data;

    if (end.success == SPAWN_SERVICE_ERROR) {
        printf("Greeter died, reexecute!\n");

        greeter->try = NULL;
        if (greeter->mode == FALLBACK)
          greeter->run_gen ++;
        greeter->mode = RUNNING;

        return;
    } else {
        printf("Greeter started.\n");

        //keep track of the session
        greeter->session = strdup(greeter->try->session);
        greeter->pid = greeter->try->pid;

        spawnregistery_listen(greeter->pid, _greeter_done, data);
    }

    greeter = NULL;
}

//runs in a seperated process
static void
_greeter_job(void *data) {
    char *cmd, *cmdpath;
    Seat_Greeter *greeter;

    greeter = data;

    if (greeter->mode == RUNNING)
        cmdpath = (char*)config->greeter.cmd;

    if (greeter->mode == FALLBACK || !cmdpath)
        cmdpath = FALLBACK_GREETER;

    cmd = basename(cmdpath);

    printf("Starting greeter app %s\n", cmdpath);
    execl(cmdpath, cmd, NULL);
    exit(1);
}

void
greeter_activate(const char *seat) {
    Seat_Greeter *greeter;

    greeter = _greeter_search(seat);
    if (!greeter) greeter = _greeter_add(seat);


    if (greeter->try) {
        session_activate(greeter->session);
        return;
    }

    if (greeter->session && greeter->pid) {
        session_activate(greeter->session);
        return;
    }

    if (greeter->run_gen == 0)
      greeter->try = spawnservice_spawn(_greeter_start_done, greeter, _greeter_job, greeter,
                                        PAM_SERVICE_GREETER, config->greeter.start_user, NULL);
}

void
greeter_lockout(const char *seat) {
    Seat_Greeter *greeter;

    greeter = _greeter_search(seat);
    greeter->end = 1;
}