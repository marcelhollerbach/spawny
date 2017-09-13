#include "main.h"

char *
session_get(pid_t pid UNUSED) {
   return "c1";
}

char*
seat_get(pid_t pid UNUSED) {
    return "seat0";
}

char*
current_session_get(void) {
    return "c1";
}

void
session_enumerate(const char *seat UNUSED, char ***handles UNUSED, unsigned int *len UNUSED) {

}

void
session_enumerate_free(char **handles UNUSED, unsigned int len UNUSED)
{

}

void
session_activate(char *handle UNUSED) {

}

bool
session_details(char *handle UNUSED, uid_t *uid UNUSED, char **name UNUSED, char **icon UNUSED, int *vtnr UNUSED) {
    return false;
}

void
wait_session_active(char *handle UNUSED) {

}
