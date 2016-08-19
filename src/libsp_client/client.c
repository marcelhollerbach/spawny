#include "Sp_Client.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define API_ENTRY(purpose_value) \
    if (!ctx) { \
        printf("Library not initializied\n"); \
        return 0; \
    }  \
    if (ctx->purpose != purpose_value) { \
        printf("Library in wrong purpose-state\n"); \
        return 0; \
    }

#define API_CALLBACK_CHECK() \
    if (!ctx) { \
        printf("Library not initializied\n"); \
        return 0; \
    }

struct _Sp_Client_Context{
   Sp_Client_Login_Purpose purpose;
   int server_sock;
   Spawny__Server__Data server_data;
};

#define WRITE(c, m) if (!_write_msg(c, m)) return false;

//Helper functions

static bool
_write_msg(Sp_Client_Context *ctx, Spawny__Greeter__Message *msg) {
    uint8_t buf[PATH_MAX];
    int len;

    len = spawny__greeter__message__pack(msg, buf);

    if (write(ctx->server_sock, buf, len) != len) {
        perror("Writing message failed");
        return false;
    }
    return true;
}

static bool
_interface_check(Sp_Client_Interface *interface) {
    if (!interface) return 0;
    if (!interface->feedback_cb) {
        printf("feedback_cb cannot be NULL\n");
        return false;
    }
    if (!interface->data_cb) {
        printf("data_cb cannot be NULL\n");
        return false;
    }
    return true;
}

//Protocoll functions

static bool
_sp_client_hello(Sp_Client_Context *ctx)
{
    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__HELLO;

    WRITE(ctx, &msg);
    return true;
}

static bool
_sp_client_start_greeter(Sp_Client_Context *ctx) {
    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__GREETER_START;
    msg.session = NULL;
    msg.login = NULL;

    WRITE(ctx, &msg);
    return true;
}


bool
sp_client_session_activate(Sp_Client_Context *ctx, char *session) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION;
    msg.session = session;

    WRITE(ctx, &msg)

    return true;
}

bool
sp_client_login(Sp_Client_Context *ctx, char *usr, char *pw, char *template) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;
    Spawny__Greeter__Login login = SPAWNY__GREETER__LOGIN__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY;
    msg.login = &login;

    msg.login->password = pw;
    msg.login->user = usr;
    msg.login->template_id = template;

    WRITE(ctx, &msg);

    return true;
}

//Context creation / free

Sp_Client_Context*
sp_client_init(Sp_Client_Login_Purpose purpose) {
    Sp_Client_Context *ctx;

    ctx = calloc(1, sizeof(Sp_Client_Context));

    if (purpose < SP_CLIENT_LOGIN_PURPOSE_START_GREETER || purpose >= SP_CLIENT_LOGIN_PURPOSE_LAST) {
        printf("Invalid purpose!\n");
        goto end;
    }

    ctx->purpose = purpose;
    ctx->server_sock = sp_service_connect();

    if (ctx->server_sock < 0) {
        //service_connect() is writing error messages
        goto end;
    }

    switch(purpose){
        case SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB:
            if (!_sp_client_hello(ctx))
                goto end;
        break;
        case SP_CLIENT_LOGIN_PURPOSE_START_GREETER:
            _sp_client_start_greeter(ctx);
            goto end;
        break;
        default:
        break;
    }

    return ctx;
end:
    if (ctx->server_sock > 0) close(ctx->server_sock);
    free(ctx);
    return NULL;
}

int
sp_client_fd_get(Sp_Client_Context *ctx)
{
   return ctx->server_sock;
}

bool
sp_client_free(Sp_Client_Context *ctx) {
    close(ctx->server_sock);
    free(ctx);

    return true;
}

//read data packages

Sp_Client_Read_Result
sp_client_read(Sp_Client_Context *ctx, Sp_Client_Interface *interface) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()
    Spawny__Server__Message *msg = NULL;
    int len = 0;
    uint8_t buf[PATH_MAX];

    if (!_interface_check(interface)) return SP_CLIENT_READ_RESULT_FAILURE;

    len = read(ctx->server_sock, buf, sizeof(buf));

    if (len < 1) {
        perror("Reading the socket connection failed");
        return SP_CLIENT_READ_RESULT_FAILURE;
    }

    msg = spawny__server__message__unpack(NULL, len, buf);

    if (!msg) {
        printf("Invalid data object\n");
        return SP_CLIENT_READ_RESULT_FAILURE;
    }

    switch(msg->type) {
        case SPAWNY__SERVER__MESSAGE__TYPE__LEAVE:
            printf("Leave msg!\n");
            return SP_CLIENT_READ_RESULT_EXIT;
        break;
        case SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE:
            printf("Data update!\n");
            ctx->server_data = *(msg->data);
            interface->data_cb(msg->data);
        break;
        case SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK:
            printf("Login Feedback!\n");
            interface->feedback_cb(msg->feedback->login_success, msg->feedback->msg);
        break;
        default:
            printf("Unknown messsage type\n");
        break;
    }
    return SP_CLIENT_READ_RESULT_SUCCESS;
}