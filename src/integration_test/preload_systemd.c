#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PUBLIC_API __attribute__ ((visibility("default")))

static bool initial_configure_check = false;
static int session_number = 0;

PUBLIC_API int
sd_pid_get_session(pid_t pid, char **session)
{
   char *s;
   if (pid == getpid() && !initial_configure_check)
     {
        initial_configure_check = true;
        s = NULL;
     }
   else
     {
        char sessionbuffer[5];

        snprintf(sessionbuffer, sizeof(sessionbuffer), "c%d", session_number);

        session_number++;

        s = strdup(sessionbuffer);
     }
     *session = s;
    return 1;
}

PUBLIC_API int
sd_seat_get_sessions(const char *seat, char ***sessions, uid_t **uid, unsigned int *n_uids)
{
   if (sessions)
     {
        char **array = calloc(3, sizeof(char*));
        array[0] = strdup("c0");
        array[1] = strdup("c1");
        array[2] = NULL;

        *sessions = array;
     }
   return 2;
}

PUBLIC_API int
sd_session_get_desktop(const char *session, char **desktop)
{
   if (desktop)
     *desktop = strdup("Ultimate Testing environment");
   return 1;
}

PUBLIC_API int
sd_session_get_seat(const char *session, char **seat)
{
    if (seat)
      *seat = strdup("seat0");
    return 1;
}

PUBLIC_API int
sd_session_get_type(const char *session, char **type)
{
   if (type)
     *type = strdup("wayland");
   return 1;
}

PUBLIC_API int
sd_session_get_uid(const char *session, uid_t *uid)
{
   //everything will be a root uid
   if (uid)
     *uid = getuid();
   return 1;
}

PUBLIC_API int
sd_session_get_vt(const char *session, unsigned int *vt)
{
   //sessions are from type c<number>
   const char *session_number = session ++;

   if (vt)
     *vt = atoi(session_number);

   return 1;
}

PUBLIC_API int
sd_session_is_active(const char *session)
{
   return 1;
}

PUBLIC_API int
__attribute__((weak)) __attribute__((visibility("default"))) _loginctl_activate(const char *handle)
{
   printf("Activate %s\n", handle);
   return 1;
}
