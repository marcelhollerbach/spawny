#include "main.h"

#include <string.h>
#include <unistd.h>
#include <systemd/sd-login.h>
#include <errno.h>
#include <limits.h>

char *
session_get(pid_t pid) {
    char *result;

    if (sd_pid_get_session(pid, &result) < 0)
      return NULL;

    return result;
}

char*
seat_get(pid_t pid) {
    char *session;
    char *result;

    session = session_get(pid);

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
    return session_get(pid);
}

void
session_enumerate(const char *seat, char ***handles, unsigned int *len) {
    int entries = sd_seat_get_sessions(seat, handles, NULL, NULL);
    if (entries < 0) {
        ERR("Failed to get sessions.");
        *handles = NULL;
        *len = 0;
        return;
    }
    *len = entries;
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
                return false; \
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

bool
session_details(char *handle, uid_t *uid, char **name, char **icon, int *vtnr) {
    int ret = -1;
    unsigned int vtnr_raw;
    char max_name[1024];
    char *tmp_desktop;
    char *tmp_type;

    GET_FIELD(sd_session_get_vt, &vtnr_raw, 0)
    if (ret < 0) {
      if (vtnr) {*vtnr = -1;}
    } else if (vtnr) {
      if (vtnr) {*vtnr = vtnr_raw;}
    }
    GET_FIELD(sd_session_get_uid, uid, 0)

    GET_FIELD(sd_session_get_desktop, &tmp_desktop, NULL)
    GET_FIELD(sd_session_get_type, &tmp_type, NULL)

    if (tmp_type && tmp_desktop)
      snprintf(max_name, sizeof(max_name), "%s - %s", tmp_type, tmp_desktop);
    else if (tmp_type)
      snprintf(max_name, sizeof(max_name), "%s", tmp_type);
    else if (tmp_desktop)
      snprintf(max_name, sizeof(max_name), "%s", tmp_desktop);

    if (tmp_desktop)
      free(tmp_desktop);
    if (tmp_type)
      free(tmp_type);

    if (name)
      *name = strdup(max_name);

    if (icon)
      *icon = NULL;
    return true;
}

void
wait_session_active(char *handle) {
    while (!sd_session_is_active(handle)) {
        sleep(1);
    }
}
