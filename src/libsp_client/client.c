#include "Sp_Client.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define API_ENTRY(purpose_value) \
    if (!context) { \
        printf("Library not initializied\n"); \
        return 0; \
    }  \
    if (context->purpose != purpose_value) { \
        printf("Library in wrong purpose-state\n"); \
        return 0; \
    }

#define API_CALLBACK_CHECK() \
    if (!context) { \
        printf("Library not initializied\n"); \
        return 0; \
    }  \
    if (!context->login_feedback_cb || !context->data_cb) { \
        printf("client_hello(...) needs to be called before using that api\n"); \
        return 0; \
    }

typedef struct {
   Sp_Client_Login_Purpose purpose;
   int server_sock;
   Sp_Client_Data_Cb data_cb;
   Sp_Client_Login_Feedback_Cb login_feedback_cb;
   Spawny__Server__Data server_data;
} Spawny_Client_Context;

static Spawny_Client_Context *context;

static void
_write_msg(Spawny__Greeter__Message *msg) {
    uint8_t buf[PATH_MAX];
    int len;

    len = spawny__greeter__message__pack(msg, buf);

    if (write(context->server_sock, buf, len) != len) {
        perror("Writing message failed");
    }
}

int
sp_client_session_activate(char *session) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION;
    msg.session = session;

    _write_msg(&msg);

    return 1;
}

int
sp_client_login(char *usr, char *pw, char *template) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;
    Spawny__Greeter__Login login = SPAWNY__GREETER__LOGIN__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY;
    msg.login = &login;

    msg.login->password = pw;
    msg.login->user = usr;
    msg.login->template_id = template;

    _write_msg(&msg);

    return 1;
}

int
sp_client_run(void) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    Spawny__Server__Message *msg = NULL;
    int len = 0;
    uint8_t buf[PATH_MAX];

    while(1) {

        len = read(context->server_sock, buf, sizeof(buf));
        if (len < 1) {
            perror("Read failed");
            return 0;
        }

        msg = spawny__server__message__unpack(NULL, len, buf);

        if (!msg) {
            printf("Invalid data package!!\n");
            continue;
        }

        switch(msg->type) {
            case SPAWNY__SERVER__MESSAGE__TYPE__LEAVE:
                printf("Leave msg!\n");
                return 1;
            break;
            case SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE:
                printf("Data update!\n");
                context->server_data = *(msg->data);
                context->data_cb(&context->server_data);
            break;
            case SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK:
                printf("Login Feedback!\n");
                context->login_feedback_cb(msg->feedback->login_success, msg->feedback->msg);
            break;
            default:
            break;
        }

        spawny__server__message__free_unpacked(msg, NULL);
    }
    return 1;
}

int
sp_client_hello(Sp_Client_Data_Cb _data_cb,
             Sp_Client_Login_Feedback_Cb _login_cb)
{
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    if (!_data_cb || !_login_cb) {
        printf("Callbacks need to be set\n");
        return 0;
    }

    context->data_cb = _data_cb;
    context->login_feedback_cb = _login_cb;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__HELLO;

    _write_msg(&msg);

    return 1;
}

int
sp_client_start_greeter(void) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_START_GREETER)

    Spawny__Greeter__Message message = SPAWNY__GREETER__MESSAGE__INIT;
    int len = 0;
    uint8_t *buf;

    message.type = SPAWNY__GREETER__MESSAGE__TYPE__GREETER_START;
    message.session = NULL;
    message.login = NULL;

    len = spawny__greeter__message__get_packed_size(&message);

    buf = malloc(len);

    spawny__greeter__message__pack(&message, buf);

    write(context->server_sock, buf, len);

    free(buf);
    return 1;
}

int
sp_client_init(Sp_Client_Login_Purpose purpose) {
    context = calloc(1, sizeof(Spawny_Client_Context));

    if (purpose < SP_CLIENT_LOGIN_PURPOSE_START_GREETER || purpose >= SP_CLIENT_LOGIN_PURPOSE_LAST) {
        printf("Invalid purpose!\n");
        return 0;
    }

    context->purpose = purpose;
    context->server_sock = sp_service_connect();

    if (context->server_sock < 0) {
        //service_connect() is writing error messages
        return 0;
    }

    return 1;
}

int
sp_client_shutdown(void) {
    close(context->server_sock);
    free(context);

    context = NULL;
    return 1;
}