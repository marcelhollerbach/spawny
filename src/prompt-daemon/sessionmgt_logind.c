#include "main.h"
#include <unistd.h>
#include <systemd/sd-login.h>

char*
current_session_get(void) {
    pid_t pid = getpid();
    char *result;

    if (sd_pid_get_session(pid, &result) < 0)
      return NULL;

    return result;
}

void
session_enumerate(char ***handles, unsigned int *len) {
    if ((*len = sd_seat_get_sessions("seat0", handles, NULL, NULL)) <= 0) {
        printf("Failed to get sessions.\n");
        *handles = NULL;
        *len = 0;
    }
}

void
session_activate(char *handle) {
    char buf[PATH_MAX];

    if (!handle) return;

    snprintf(buf, sizeof(buf), "loginctl activate %s", handle);

    system(buf);
}

int
session_details(char *handle, uid_t *uid, char **name, char **icon) {
    if (sd_session_get_uid(handle, uid) < 0 ||
        sd_session_get_class(handle, name) < 0)
        return 0;
    *icon = NULL;
    return 1;
}

void
wait_session_active(char *handle) {
    while (!sd_session_is_active(handle)) {
        sleep(1);
    }
}