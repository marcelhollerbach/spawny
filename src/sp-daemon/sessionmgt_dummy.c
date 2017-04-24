#include "main.h"

char *
session_get(pid_t pid) {
   return "c1";
}

char*
seat_get(pid_t pid) {
    return "seat0";
}

char*
current_session_get(void) {
    return "c1";
}

void
session_enumerate(const char *seat, char ***handles, unsigned int *len) {

}

void
session_enumerate_free(char **handles, unsigned int len)
{

}

void
session_activate(char *handle) {

}

bool
session_details(char *handle, uid_t *uid, char **name, char **icon, int *vtnr) {
    return false;
}

void
wait_session_active(char *handle) {

}
