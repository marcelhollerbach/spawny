#include "main.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stropts.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>

#include <security/pam_appl.h>

#define _XOPEN_SOURCE 500
#include <termios.h>

#define WRITE 1
#define READ 0

#define LAST_TTY 63
#define FIRST_TTY 10

static int tty_counter = FIRST_TTY;

static int child_run(Spawn_Try *try);
static int open_tty(Spawn_Try *try, char *tty);
static int pam_auth(Spawn_Try *try, char ***env, int tty);

static void _spawned_session_disappear(void *data, int signal, pid_t pid);

static void
_spawn_try_free(Spawn_Try *try)
{
    Spawn_Service_End end;

    close(try->com.fd[READ]);
    close(try->com.fd[WRITE]);

    end.success = try->exit.success;
    end.message = try->exit.error_msg;

    if (try->done.cb)
      try->done.cb(try->done.data, end);

    if (try->exit.error_msg)
      free(try->exit.error_msg);

    if (try->session)
      free(try->session);

    spawnregistery_unlisten(try->pid, _spawned_session_disappear, try);
    try->pid = -1;

    free(try);
}

static void
_child_data(Fd_Data *data, int fd)
{
   Spawny__Spawn__Message *msg = NULL;
   Spawn_Try *try = data->data;
   uint8_t buffer[PATH_MAX];
   int len;

   len = read(fd, buffer, sizeof(buffer));

   msg = spawny__spawn__message__unpack(NULL, len, buffer);

   if (!msg) {
     printf("Got a wrong message.\n");
     return;
   }

   switch(msg->type) {
       case SPAWNY__SPAWN__MESSAGE__TYPE__CONNECT:
           //FIXME kill timer for none come up sessions
       break;
       case SPAWNY__SPAWN__MESSAGE__TYPE__SETUP_DONE:
           //we need to activate the passed session
           try->session = sesison_get(try->pid);
           printf("Activating session %s\n", try->session);
           session_activate(try->session);
       break;
       case SPAWNY__SPAWN__MESSAGE__TYPE__SESSION_ACTIVE:
           //the session is up, and will die now
           printf("Session is up\n");
           manager_unregister_fd(try->com.fd[READ]);
           try->exit.success = SPAWN_SERVICE_SUCCESS;
           _spawn_try_free(try);
       break;
       case SPAWNY__SPAWN__MESSAGE__TYPE__ERROR_EXIT:
           //print the error somewhere
           printf("Session exit with %s\n", msg->exit);
           manager_unregister_fd(try->com.fd[READ]);
           try->exit.error_msg = strdup(msg->exit);
           _spawn_try_free(try);
       break;
       default:
       break;
   }
   spawny__spawn__message__free_unpacked(msg, NULL);
}

Spawn_Try*
spawnservice_spawn(SpawnDoneCb done, void *data,
                   SpawnServiceJobCb job, void *jobdata,
                   const char *service, const char *usr, const char *pw) {

    Spawn_Try *spawn_try = calloc(1, sizeof(Spawn_Try));

    //setting it to error, if the process dies before sending anything, a error should be emitted
    spawn_try->exit.success = SPAWN_SERVICE_ERROR;

    if (pipe(spawn_try->com.fd) < 0) {
        perror("Failed to open pipe");
        free(spawn_try);
        spawn_try = NULL;
        return 0;
    }

    spawn_try->job.cb = job;
    spawn_try->job.data = jobdata;

    spawn_try->pw = pw;
    spawn_try->usr = usr;
    spawn_try->service = service;

    spawn_try->done.cb = done;
    spawn_try->done.data = data;

    //this will start a client process which is in a new session
    //this will also register the read fd to the manager
    if (!child_run(spawn_try)) {
        _spawn_try_free(spawn_try);
    }

    return spawn_try;
}

void
_child_exit(int signal)
{
   if (signal != SIGCHLD) return;

   spawnregistery_eval();
}

void
spawnservice_init(void)
{
   signal(SIGCHLD, _child_exit);
}

//=====================================================
//BE AWARE
//the following stuff is 99% into a seperated process,
//=====================================================

static char
_tty_used(char *buf) {
    unsigned int number;
    char **sessions;

    session_enumerate(&sessions, &number);
    for(int i = 0; i < number; i++){
        char *tty = NULL;

        session_details(sessions[i], NULL, NULL, NULL, &tty);
        if (!tty) continue;

        if (!strcmp(tty, buf))
          return 1;
    }
    return 0;
}

#define DEVICE_PREFIX "/dev/"

static char *
available_tty(void) {
    char buf[PATH_MAX];

    do {
        if (tty_counter > LAST_TTY) return NULL;
        tty_counter ++;

        snprintf(buf, sizeof(buf), DEVICE_PREFIX"tty%d", tty_counter);
    } while(_tty_used(buf + sizeof(DEVICE_PREFIX) - 1));

    return strdup(buf);
}

static void
_service_message(Spawn_Try *try, Spawny__Spawn__MessageType type, const char *mmessage, char *session) {
    int len = 0;
    void *data;
    Spawny__Spawn__Message message = SPAWNY__SPAWN__MESSAGE__INIT;

    message.exit = (char*)mmessage;
    message.type = type;

    len = spawny__spawn__message__get_packed_size(&message);
    data = malloc(len);

    spawny__spawn__message__pack(&message, data);

    if (write(try->com.fd[WRITE], data, len) != len)
      perror("Failed to write service message");
    free(data);
}

static void
_service_com_exit(Spawn_Try *try, const char *reason) {
    _service_message(try, SPAWNY__SPAWN__MESSAGE__TYPE__ERROR_EXIT, reason, 0);
}

static void
_session_proc_fire_up(Spawn_Try *try) {
    pid_t pid;

    pid = fork();

    if (pid == 0) {
        //call the client function*/
        if (try->job.cb)
          try->job.cb(try->job.data);
    } else if (pid == -1) {
        perror("fork failed");
    } else {
        waitpid(pid, NULL, 0);
    }
    exit(1);
}

static void
_spawned_session_disappear(void *data, int signal, pid_t pid)
{
   Spawn_Try *try;

   try = data;

   _spawn_try_free(try);
}

static int
child_run(Spawn_Try *try){
    pid_t pid;
    char *tty;

    //check available ttys
    tty = available_tty();

    if (!tty) {
        _service_com_exit(try, "Failed to find tty");
    }

    pid = fork();

    if (pid == -1) {
        // something went wrong
        return 0;
    } else if (pid == 0) {
        //we are the child
        char *session;
        char **env;

        //we must be our own session
        setsid();

        //close the read fd
        close(try->com.fd[READ]);

        //connect
        _service_message(try, SPAWNY__SPAWN__MESSAGE__TYPE__CONNECT, NULL, NULL);

        //we are the child
        close(STDOUT_FILENO);
        close(STDIN_FILENO);
        close(STDERR_FILENO);

        //open tty
        if (!open_tty(try, tty)) {
             //open tty allready said that we are dead
             exit(-1);
          }


        //log into pam
        if (!pam_auth(try, &env, tty_counter)) {
             exit(-1);
          }

        clearenv();
        //set all environment variables
        for (int i = 0; env[i]; i++) {
            putenv(env[i]);
        }
        setenv("TERM", "linux", 0);

        //get the new session
        session = current_session_get();

        if (!session) {
           _service_com_exit(try, "session is not valid");
           exit(-1);
        }

        //tell service that we are alive and happy
        //the service should activate this session next
        _service_message(try, SPAWNY__SPAWN__MESSAGE__TYPE__SETUP_DONE, NULL, session);

        //wait for the session to get active
        wait_session_active(session);

        _service_message(try, SPAWNY__SPAWN__MESSAGE__TYPE__SESSION_ACTIVE, NULL, NULL);

        //close child fd
        close(try->com.fd[WRITE]);


        _session_proc_fire_up(try);


        //if we get here we are basically fucked.
        printf("Job didnt take the process, exiting!\n");

        exit(-1);
    } else {
        //close the write end of the pipe, we will just hear
        close(try->com.fd[WRITE]);
        //register the read fd to the manager so we will get notified if the client sents something
        manager_register_fd(try->com.fd[READ], _child_data, try);
        //listen for the process to disappear
        spawnregistery_listen(pid, _spawned_session_disappear, try);
        try->pid = pid;
        return 1;
    }
}

//open tty stuff
static int
open_tty(Spawn_Try *try, char *tty)
{
   static struct sigaction sa, osa;
   struct group *gr = NULL;
   struct passwd *pwd= NULL;
   pid_t tid, pid = getpid();
   int fd;


   sa.sa_handler = SIG_IGN;
   sa.sa_flags = SA_RESTART;

   sigaction(SIGHUP, &sa, &osa);

   /* check if user is found */
   pwd = getpwnam(try->usr);
   if (!pwd) {
        _service_com_exit(try, "Failed to fetch user");
        return 0;
   }

   /* fetch the tty group */
   gr = getgrnam("tty");
   if (!gr)
     {
        _service_com_exit(try, "Failed to get group tty!");
        return 0;
     }

   /* open the tty */
   if ((fd = open(tty, O_RDWR|O_NOCTTY|O_NONBLOCK, 0)) < 0)
     {
        _service_com_exit(try, "Failed to open tty");
        return 0;
     }

   /* check if this is really a tty */
   if (!isatty(fd))
     {
        _service_com_exit(try, "FD is not a tty");
        return 0;
     }

   // check if we are the controlling process
   if (((tid = tcgetsid(fd)) < 0) || (pid != tid))
     {
        /* if not try to get it */
        if (ioctl(fd, TIOCSCTTY) == -1)
          {
             _service_com_exit(try, "Getting control of tty failed");
             //CM("Getting Controlling TTY Failed, Reason: %s", strerror(errno));
             return 0;
          }
     }
   // check if this is really fd 0, 1, 2 if not we have a problem
   if (fd != 0 || dup(fd) != 1 || dup(fd) != 2)
     {
        _service_com_exit(try, "TTY fd´s where not at place 0, 1 and 2");
        return 0;
     }

    //reset tty
    if (vhangup() < 0) {
        _service_com_exit(try, "vhangup failed!\n");
        return 0;
    }

    /* close everything */
    close(STDOUT_FILENO);
    close(STDIN_FILENO);
    close(STDERR_FILENO);

    /* Now we reopen every tty*/
    /* open the tty */
    if ((fd = open(tty, O_RDWR|O_NOCTTY, 0)) < 0) {
        _service_com_exit(try, "Failed to open tty");
        return 0;
    }

    // check if we are the controlling process
    if (((tid = tcgetsid(fd)) < 0) || (pid != tid)) {
        /* if not try to get it */
        if (ioctl(fd, TIOCSCTTY) == -1) {
             _service_com_exit(try, "Getting control of tty failed");
             //CM("Getting Controlling TTY Failed, Reason: %s", strerror(errno));
             return 0;
          }
     }

    //reset tty
    tcflush(STDIN_FILENO, TCIFLUSH);

    if (tcsetpgrp(STDIN_FILENO, getpid())) {
        _service_com_exit(try, "Failed to get foreground proccess");
        return 0;
    }

    // check if this is really fd 0, 1, 2 if not we have a problem
    if (fd != 0 || dup(fd) != 1 || dup(fd) != 2) {
        _service_com_exit(try, "TTY fd´s where not at place 0, 1 and 2");
        return 0;
    }

    /* own the tty device */
    if (chown(tty, pwd->pw_uid, gr->gr_gid) || chmod(tty, (gr->gr_gid ? 0620 : 0600))) {
        _service_com_exit(try, "Own and mod failed!!!");
        return 0;
    }

    sigaction(SIGHUP, &osa, NULL);

    /* clear everything */
    printf("\033[r\033[H\033[J");

    printf("\\o/ tty is alive!\n");

   return 1;
}

//pam stuff follows

static int
_pam_conv(int num_msg, const struct pam_message **msg,
         struct pam_response **rp, void *appdata_ptr) {
   Spawn_Try *try = appdata_ptr;
   int i = 0;

   *rp = (struct pam_response *) calloc(num_msg, sizeof(struct pam_response));
   for (i = 0; i < num_msg; i++)
     {
        rp[i]->resp_retcode = 0;
        switch(msg[i]->msg_style)
          {
             case PAM_PROMPT_ECHO_ON:
                //_service_com_exit(msg[i]->msg);
                break;
             case PAM_PROMPT_ECHO_OFF:
                rp[i]->resp = try->pw ? strdup(try->pw) : NULL;
                break;
             case PAM_ERROR_MSG:
                _service_com_exit(try, msg[i]->msg);
                break;
             case PAM_TEXT_INFO:
                _service_com_exit(try, msg[i]->msg);
                break;
             default:

                break;
          }
     }
   return PAM_SUCCESS;
}

static void
_handle_error(Spawn_Try *try, int ret) {
    #define RET_VAL_REG(v, val) \
    case v: \
        _service_com_exit(try, "Pam error "val" happend"); \
        break;

    switch(ret){
        RET_VAL_REG(PAM_BUF_ERR, "PAM_BUF_ERR");
        RET_VAL_REG(PAM_CONV_ERR, "PAM_CONF_ERR");
        RET_VAL_REG(PAM_ABORT, "PAM_ABORT");
        RET_VAL_REG(PAM_SYSTEM_ERR, "PAM_SYSTEM_ERR");
        RET_VAL_REG(PAM_AUTH_ERR, "PAM_AUTH_ERR");
        RET_VAL_REG(PAM_CRED_INSUFFICIENT, "PAM_CRED_INSUFFICIENT");
//FIXME WTF THIS IS FROM THE DOC         RET_VAL_REG(PAM_AUTHINFO_UNVAIL, "PAM_AUTHINFO_UNVAIL");
        RET_VAL_REG(PAM_MAXTRIES, "PAM_MAX_TRIES");
        RET_VAL_REG(PAM_USER_UNKNOWN, "PAM_USER_UNKNOWN");
        default:
            _service_com_exit(try, "uncaught state");
        break;
    }
    #undef RET_VAL_REG
}

static int
pam_auth(Spawn_Try *try, char ***env, int vtnr) {
    int ret = 0;
    pam_handle_t *handle;
    struct pam_conv conv;
    struct passwd *pwd;

    conv.conv = _pam_conv;
    conv.appdata_ptr = try;
    pwd = getpwnam(try->usr);

#define PAM_CHECK \
      if (ret != PAM_SUCCESS) { \
        _handle_error(try, ret); \
        return 0; \
      }

    ret = pam_start(try->service, try->usr, &conv, &handle);
    PAM_CHECK

#define PE(k,v) { \
     char buf[PATH_MAX]; \
     snprintf(buf, sizeof(buf), "%s=%s", k, v); \
     ret = pam_putenv(handle, buf); \
     PAM_CHECK }

    PE("HOME",pwd->pw_dir);
    PE("SHELL",pwd->pw_shell);
    PE("USER",pwd->pw_name);
    PE("LOGNAME",pwd->pw_name);
    PE("XDG_SEAT", "seat0");
#undef PE
#define PE(k,v) { \
 char buf[PATH_MAX]; \
 snprintf(buf, sizeof(buf), "%s=%d", k, v); \
 ret = pam_putenv(handle, buf); \
 PAM_CHECK }
   PE("XDG_VTNR", vtnr);

#undef PE

    ret = pam_authenticate(handle, 0);
    PAM_CHECK
    ret = pam_setcred(handle, PAM_ESTABLISH_CRED);
    PAM_CHECK
    ret = pam_open_session(handle, 0);
    PAM_CHECK

#undef PAM_CHECK

    *env = pam_getenvlist(handle);

    if (initgroups(try->usr, pwd->pw_gid) != 0) {
        _service_com_exit(try, "init the groups of usr failed");
        return 0;
    }
    if (setgid(pwd->pw_gid) != 0) {
        _service_com_exit(try, "failed to set gid");
        return 0;
    }
    if (setuid(pwd->pw_uid) != 0) {
        _service_com_exit(try, "failed to set uid");
        return 0;
    }
    return 1;
}
