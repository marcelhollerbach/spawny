#ifndef SESSIONMGT_H
#define SESSIONMGT_H

#include <unistd.h>

/**
 * This header describes the interface between
 * the spawny daemon and the session management
 * systems that could be implemented.
 *
 * Strings returned by the functions needs to be \0-terminated.
 * The strings are also outputted with printf("%s", ... );
 */

/**
 * Get the session of a given pid
 *
 * @param pid the pid to get the session from
 *
 * @return the handle to the session or NULL on failure
 */
char *
session_get(pid_t pid);

/**
 * Get the seat of a given pid
 *
 * @param pid the pid to egt the seat from
 *
 * @return the handle to the seat or NULL on failure
 */
char *
seat_get(pid_t pid);

/**
 * Get the current session.
 */
char *
current_session_get(void);

/**
 * Enumerate the sessions for a given seat
 *
 * @param seat the seat to get the sessions from
 * @param handles A pointer to store a string array.
 *                This handle needs to be freeed by the caller.
 * @param len A pointer to store a unsigned int in.
 *            The int describes the lengh of the array in *handles
 */
void
session_enumerate(const char *seat, char ***handles, unsigned int *len);

void
session_enumerate_free(char **handles, unsigned int len);
/**
 * Activate the given session
 *
 * @param handle The session to activate
 */
void
session_activate(char *handle);

/**
 * Get details of a session
 *
 * @param handle the session to get the details from
 * @param uid pointer to store the uid in of a session
 * @param names pointer to store the string of the running name
 * @param icon pointer to store the string describing the icon of the session
 * @param vtnr pointer to store the virtual terminal number in
 *
 * @return true on sucess, false if get fetching failed
 */
bool
session_details(char *handle, uid_t *uid, char **name, char **icon, int *vtnr);

/**
 * This function is called from a seperated process!
 *
 * The function should block until the given session is activated
 *
 * @param session The session to wait for
 *
 */
void
wait_session_active(char *session);

#endif
