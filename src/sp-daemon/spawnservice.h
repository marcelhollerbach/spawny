#ifndef SPAWNSERVICE_H
#define SPAWNSERVICE_H

typedef enum {
    SPAWN_SERVICE_SUCCESS = 0,
    SPAWN_SERVICE_ERROR = 1,
} Spawn_Service_State;

typedef struct {
    Spawn_Service_State success; //0 means error, 1 means success
    const char *message; //message for the case of an error
} Spawn_Service_End;

typedef void (*SpawnDoneCb)(void *data, Spawn_Service_End success);
typedef void (*SpawnServiceJobCb)(void *data);

typedef struct {
    const char *session_type;
    const char *session_desktop;
    const char *session_seat;
} Xdg_Settings;

typedef struct _Spawn_Try{
    struct {
        int fd[2];
    } com;
    struct {
        SpawnServiceJobCb cb;
        void *data;
    } job, session_ended;
    struct {
        SpawnDoneCb cb;
        void *data;
    } done;
    const char *service;
    const char *usr;
    const char *pw;

    char *session;
    struct {
      char *error_msg;
      Spawn_Service_State success;
    } exit;

    pid_t pid;
    Xdg_Settings settings;
} Spawn_Try;

/*
 * Spawns a new job
 * the lib can always just spawn new jobs if no other is in starting mode
 * If a nother is starting right now 0 is returned
 *
 * @param done callback called in the process when job is executed
 * @param data data passed to done
 * @param job the job to execute in a new session,
 *        be carefull, this job will run in a complete
 *        undepended process from the calling process.
 * @param data passed field to the job
 * @param service name of the pam service to use
 * @param user username to use as user
 * @param pw password used in pam

 */
Spawn_Try*
spawnservice_spawn(SpawnDoneCb done, void *data,
                   SpawnServiceJobCb job, void *jobdata,
                   SpawnServiceJobCb session_ended, void *session_ended_data,
                   const char *service, const char *usr, const char *pw, Xdg_Settings *sessiongs);

void spawnservice_init(void);

#endif
