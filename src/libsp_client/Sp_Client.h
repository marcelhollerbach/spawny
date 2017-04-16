#ifndef SOCKET_CREATION_H
#define SOCKET_CREATION_H

#include <stdbool.h>

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

/*
 * Generic type for communcating with arrays with a length
 */

typedef struct {
  int length;
  void *data;
} Numbered_Array;

/**
 * Accesses a given array struct,
 *
 * @param array should be a pointer
 *
 * @param type the type which is saved in the array
 *
 * @param pos the position which should be accessed (NOT bound checked)
 */
#define ARRAY_ACCESS(array, type, pos) ((type*)(array)->data)[pos]

typedef struct {
  int id;
  char *icon;
  char *name;
} Template;

#define TEMPLATE_ARRAY(array, pos) ARRAY_ACCESS(array, Template, pos)

typedef struct {
  int id;
  char *icon;
  char *name;
  int prefered_session; //id of a template
} User;

#define USER_ARRAY(array, pos) ARRAY_ACCESS(array, User, pos)

typedef struct {
  int id;
  char *icon;
  char *user;
  char *name;
} Session;

#define SESSION_ARRAY(array, pos) ARRAY_ACCESS(array, Session, pos)

typedef void (*Sp_Client_Data_Cb)(void);
typedef void (*Sp_Client_Feedback_Cb)(int succes, char *msg);

typedef struct {
    Sp_Client_Data_Cb data_cb;
    Sp_Client_Feedback_Cb feedback_cb;
} Sp_Client_Interface;

typedef struct _Sp_Client_Context Sp_Client_Context;

/**
 * Create a new context. Will scan the argv for --debug or -d to enable connecting to the debug deamon
 *
 * @param argv    the arguments that are passed to the application
 * @param argc    the arguments count that got passed to the application
 * @param purpose if purpose is SP_CLIENT_LOGIN_PURPOSE_START_GREETER
                  the function ALWAYS returns NULL and sends the start
                  greeter to the daemon
                  Otherwise it connects to the client and waits for commands.
 *
 * @return a new context on success NULL on failure if purpose is not START_GREETER.
 *
 */
Sp_Client_Context* sp_client_init(int argc, char *argv[], Sp_Client_Login_Purpose purpose);

/**
 * Get the data. Can return empty stuff if nothing is here right now
 *
 * @param ctx Context to use
 *
 * @param sessions pointer to a valid Numbered_Array instance, which will be filled.
 *
 * @param templates same as sessions just for templates
 *
 * @param templates same as sessions just for users
 */
void sp_client_data_get(Sp_Client_Context *ctx, Numbered_Array *sessions, Numbered_Array *templates, Numbered_Array *users);

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
 * @param template id of the template to use to start
 *
 * @return 0 on faliure 1 on success
 */
bool sp_client_login(Sp_Client_Context *ctx, const char *usr, const char *pw, int template);

/**
 * Sent a Session activation command to the daemon.
 *
 * No more api-calls are possible after that.
 *
 * @param ctx Context to use
 *
 * @param session The session id to activate
 *
 * @return 0 on faliure 1 on success
 */
bool sp_client_session_activate(Sp_Client_Context *ctx, int session);

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

#endif
