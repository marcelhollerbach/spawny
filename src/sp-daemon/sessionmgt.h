#ifndef SESSIONMGT_H
#define SESSIONMGT_H

#include <unistd.h>

char * session_get(pid_t pid);
char* seat_get(pid_t pid);
char* current_session_get(void);

void session_enumerate(const char *seat, char ***handles, unsigned int *len);

void wait_session_active(char *handle);
void session_activate(char *handle);
bool session_details(char *handle, uid_t *uid, char **name, char **icon, int *vtnr);

#endif
