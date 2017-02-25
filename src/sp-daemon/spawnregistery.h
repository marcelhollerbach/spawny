#ifndef SPAWNREGISTERY_H
#define SPAWNREGISTERY_H

#include <sys/types.h>
#include <sys/wait.h>

void spawnregistery_listen(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data);
void spawnregistery_unlisten(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data);

/*this will call wait and check what is going on */
void spawnregistery_eval(void);

#endif
