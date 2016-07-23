#ifndef SOCKET_CREATION_H
#define SOCKET_CREATION_H

#include <sys/un.h>

#include "greeter.pb-c.h"
#include "server.pb-c.h"

typedef enum {
    SP_CLIENT_LOGIN_PURPOSE_START_GREETER = 0,
    SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB = 1,
    SP_CLIENT_LOGIN_PURPOSE_LAST = 2
} Sp_Client_Login_Purpose;

typedef void (*Sp_Client_Data_Cb)(Spawny__Server__Data *data);
typedef void (*Sp_Client_Login_Feedback_Cb)(int succes, char *msg);

/**
 * Init the library for the purpose passed in
 *
 * @param purpose if purpose is SPAWNY_CLIENT_LOGIN_PURPOSE_START_GREETER
 *                only client_start_greeter is valid
 * @return 0 on failure 1 on success
 *
 */
int sp_client_init(Sp_Client_Login_Purpose purpose);

/**
 * Shutdown the library
 * Apis can only be called after another init call
 */
int sp_client_shutdown(void);

/**
 * Setup the callbacks and sent the hello messages to the daemon
 *
 * @param _data_cb The callback which is called when data arrives from the daemon
 *
 * @param _login_cb The callback is called after feedback of a client_login arrives
 *
 * @return 0 on failure 1 on success
 */
int sp_client_hello(Sp_Client_Data_Cb _data_cb, Sp_Client_Login_Feedback_Cb _login_cb);

/**
 * Sent a Login message to the daemon
 *
 * @param usr must be a valid user which was offered by _data_cb
 *
 * @param pw the password for the user to auth with
 *
 * @param template the string must be one of the handles submitted by the data callback
 *
 * @return 0 on faliure 1 on success
 */
int sp_client_login(char *usr, char *pw, char *template);

/**
 * Sent a Session activation command to the daemon.
 *
 * No more api-calls are possible after that.
 *
 * @param session the session to activate, the session handle should be from the data callback
 *
 * @return 0 on faliure 1 on success
 */
int sp_client_session_activate(char *session);

/**
 * Run the polling loop to get events.
 */
int sp_client_run(void);

/**
 * Sent a start greeter message
 *
 * This will either start a new greeter or activate the session of a running one
 */
int sp_client_start_greeter(void);

/**
 * Apis to get access to the service socket
 */

const char* sp_service_path_get(void);
int sp_service_socket_create(void);
void sp_service_address_setup(struct sockaddr_un *in);
int sp_service_connect(void);

#endif