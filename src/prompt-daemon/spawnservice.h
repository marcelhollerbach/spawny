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

/*
 * Init spawn part, which can be used to start sessions
 *
 * @param done called each time a spawn service was finished
 * @param data passed to the done callback
 */
int spawnservice_init(SpawnDoneCb done, void *data);

/*
 * Spawns a new job
 * the lib can always just spawn new jobs if no other is in starting mode
 * If a nother is starting right now 0 is returned
 *
 * @param job the job to execute in a new session,
 *        be carefull, this job will run in a complete
 *        undepended process from the calling process.
 * @param service name of the pam service to use
 * @param user username to use as user
 * @param pw password used in pam
 * @param data passed field to the job
 */
int spawnservice_spawn(SpawnServiceJobCb job, const char *service,
    const char *usr, const char *pw, void *data);

/**
 * Shutdown and free all resources
 */
void spawnservice_shutdown(void);

#endif