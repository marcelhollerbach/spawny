#ifndef SOCKET_CREATION_H
#define SOCKET_CREATION_H

#include <sys/un.h>
#include <stdbool.h>

#include "greeter.pb-c.h"
#include "server.pb-c.h"

typedef enum {
    SP_CLIENT_LOGIN_PURPOSE_START_GREETER = 0,
    SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB = 1,
    SP_CLIENT_LOGIN_PURPOSE_LAST = 2
} Sp_Client_Login_Purpose;

typedef enum {
    SP_CLIENT_READ_RESULT_SUCCESS = 0,
    SP_CLIENT_READ_RESULT_FAILURE = 1,
    SP_CLIENT_READ_RESULT_EXIT = 2,
    SP_CLIENT_READ_RESULT_LAST = 3
} Sp_Client_Read_Result;

typedef void (*Sp_Client_Data_Cb)(Spawny__Server__Data *data);
typedef void (*Sp_Client_Login_Feedback_Cb)(int succes, char *msg);

typedef struct {
    Sp_Client_Data_Cb data_cb;
    Sp_Client_Login_Feedback_Cb feedback_cb;
} Sp_Client_Interface;

typedef struct _Sp_Client_Context Sp_Client_Context;

/**
 * Create a new context
 *
 * @param purpose if purpose is SP_CLIENT_LOGIN_PURPOSE_START_GREETER
                  the function ALWAYS returns NULL and sends the start
                  greeter to the daemon
                  Otherwise it connects to the client and waits for commands.
 *
 * @return a new context on success NULL on failure if purpose is not START_GREETER.
 *
 */
Sp_Client_Context* sp_client_init(Sp_Client_Login_Purpose purpose);

/**
 * Get the fd from the context
 */
int sp_client_fd_get(Sp_Client_Context *ctx);

/**
 * Free the context
 *
 */
bool sp_client_free(Sp_Client_Context *ctx);

/**
 * Sent a Login message to the daemon
 *
 * @param ctx Context to use
 *
 * @param usr must be a valid user which was offered by _data_cb
 *
 * @param pw the password for the user to auth with
 *
 * @param template the string must be one of the handles submitted by the data callback
 *
 * @return 0 on faliure 1 on success
 */
bool sp_client_login(Sp_Client_Context *ctx, const char *usr, const char *pw, const char *template);

/**
 * Sent a Session activation command to the daemon.
 *
 * No more api-calls are possible after that.
 *
 * @param ctx Context to use
 *
 * @param session the session to activate, the session handle should be from the data callback
 *
 * @return 0 on faliure 1 on success
 */
bool sp_client_session_activate(Sp_Client_Context *ctx, char *session);

/**
 * Read from the context fd and call the callbacks according to the interfaces struct.
 *
 * @param ctx Context to use
 *
 * @param interface to call for callbacks
 *
 * @return SUCCESS on success EXIT if the daemon requests to exit and FAILURE if the read fails
 */
Sp_Client_Read_Result sp_client_read(Sp_Client_Context *ctx, Sp_Client_Interface *interface);

/**
 * Apis to get access to the service socket
 */

const char* sp_service_path_get(void);
int sp_service_socket_create(void);
void sp_service_address_setup(struct sockaddr_un *in);
int sp_service_connect(void);

#endif