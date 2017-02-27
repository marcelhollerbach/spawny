#include "main.h"
#include <unistd.h>
#include <systemd/sd-login.h>
#include <errno.h>

char *
sesison_get(pid_t pid) {
    char *result;

    if (sd_pid_get_session(pid, &result) < 0)
      return NULL;

    return result;
}

char*
seat_get(pid_t pid) {
    char *session;
    char *result;

    session = sesison_get(pid);

    if (!session) return NULL;
    if (sd_session_get_seat(session, &result) < 0) {
        result = NULL;
    }
    free(session);
    return result;
}

char*
current_session_get(void) {
    pid_t pid = getpid();
    return sesison_get(pid);
}

void
session_enumerate(char ***handles, unsigned int *len) {
    if ((*len = sd_seat_get_sessions("seat0", handles, NULL, NULL)) <= 0) {
        ERR("Failed to get sessions.");
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

#define GET_FIELD(func, field, errval) \
    if (field) { \
        ret = func(handle, field); \
        switch(ret){ \
            case -ENXIO: \
                INF("Error, The specified session does not exist."); \
                *field = errval; \
            break; \
            case -ENODATA: \
                INF("Error, The given field is not specified for the described session."); \
                *field = errval; \
            break; \
            case -EINVAL: \
                INF("Error, An input parameter was invalid (out of range, or NULL, where that is not accepted)."); \
                *field = errval; \
            break; \
            case -ENOMEM: \
                INF("Error, Memory allocation failed."); \
                *field = errval; \
            break; \
        }\
    }

int
session_details(char *handle, uid_t *uid, char **name, char **icon, char **tty) {
    int ret;

    GET_FIELD(sd_session_get_tty, tty, NULL)
    GET_FIELD(sd_session_get_uid, uid, 0)
    GET_FIELD(sd_session_get_desktop, name, NULL)

    if (icon)
      *icon = NULL;
    return 1;
}

void
wait_session_active(char *handle) {
    while (!sd_session_is_active(handle)) {
        sleep(1);
    }
}
