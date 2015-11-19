#ifndef SPAWNSERVICE_H
#define SPAWNSERVICE_H

typedef void (*SpawnDoneCb)(void *data, int success);
typedef void (*SpawnServiceJobCb)(void *data);

//this inits the service done is called each time a job was successfull or not
int spawnservice_init(SpawnDoneCb done, void *data);

//this spawns a new job, if there is allready a job which needs to be spawned this funktion returns 0
int spawnservice_spawn(SpawnServiceJobCb job, const char *service,
    const char *usr, const char *pw, void *data);

void spawnservice_shutdown(void);

#endif