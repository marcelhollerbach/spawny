#include "Sp_Client.h"
#include "sp_client_private.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char *session_id;
    int id;
} Private_Session;

typedef struct {
    char *template_id;
    int id;
} Private_Template;

static void _data_convert(Sp_Client_Context *ctx, Spawny__Server__Data *data);

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
   Numbered_Array sessions;
   Numbered_Array private_session;
   Numbered_Array templates;
   Numbered_Array private_templates;
   Numbered_Array users;
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
sp_client_session_activate(Sp_Client_Context *ctx, int session) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    if (session < 0 || session >= ctx->private_session.length) return false;

    Private_Session ps = ARRAY_ACCESS(&ctx->private_session, Private_Session, session);

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION;
    msg.session = (char*) ps.session_id;

    WRITE(ctx, &msg)

    return true;
}

bool
sp_client_login(Sp_Client_Context *ctx, const char *usr, const char *pw, int template) {
    API_ENTRY(SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB)
    API_CALLBACK_CHECK()

    if (template < 0 || template >= ctx->private_templates.length) return false;

    Private_Template pt = ARRAY_ACCESS(&ctx->private_templates, Private_Template, template);

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;
    Spawny__Greeter__Login login = SPAWNY__GREETER__LOGIN__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY;
    msg.login = &login;

    msg.login->password = (char*) pw;
    msg.login->user = (char*) usr;
    msg.login->template_id = (char*) pt.template_id;

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
            _data_convert(ctx, msg->data);
            interface->data_cb();
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

void
sp_client_data_get(Sp_Client_Context *ctx, Numbered_Array *sessions, Numbered_Array *templates, Numbered_Array *users)
{
    if (sessions) memcpy(sessions, &ctx->sessions, sizeof(Numbered_Array));
    if (templates) memcpy(templates, &ctx->templates, sizeof(Numbered_Array));
    if (users) memcpy(users, &ctx->users, sizeof(Numbered_Array));
}


#define FREE_S(n) if (n) free(n); \
                  n = NULL;

#define STRDUP_S(n, v) if (n) v = strdup(n);

static void
_template_convert(void *data2, void *goal, int id)
{
    Spawny__Server__SessionTemplate *data = data2;
    Template *template = goal;

    template->id = id;
    STRDUP_S(data->icon, template->icon);
    STRDUP_S(data->name, template->name);
}

static void
_template_free(void *data)
{
    Template *template = data;

    FREE_S(template->name);
    FREE_S(template->icon);
}

static void
_session_convert(void *data2, void *goal, int id)
{
    Spawny__Server__Session *data = data2;
    Session *session = goal;

    session->id = id;
    STRDUP_S(data->icon, session->icon);
    STRDUP_S(data->name, session->name);
    STRDUP_S(data->user, session->user);
}

static void
_session_free(void *data)
{
    Session *session = data;

    FREE_S(session->icon);
    FREE_S(session->name);
    FREE_S(session->user);
}

static void
_user_convert(void *data2, void *goal, int id)
{
    Spawny__Server__User *data = data2;
    User *user = goal;

    user->id = id;
    STRDUP_S(data->icon, user->icon);
    STRDUP_S(data->name, user->name);
}

static void
_user_free(void *data)
{
    User *user = data;

    FREE_S(user->icon);
    FREE_S(user->name);
}

static void
_private_session_convert(void *data2, void *goal, int id)
{
    Spawny__Server__Session *data = data2;
    Private_Session *session = goal;

    session->id = id;
    STRDUP_S(data->id, session->session_id);
    printf("%s\n", data->id);
}

static void
_private_session_free(void *data)
{
    Private_Session *session = data;

    FREE_S(session->session_id);
}

static void
_private_template_convert(void *data2, void *goal, int id)
{
    Spawny__Server__SessionTemplate *data = data2;
    Private_Template *template = goal;

    template->id = id;
    STRDUP_S(data->id, template->template_id);
}

static void
_private_template_free(void *data)
{
    Private_Template *template = data;

    FREE_S(template->template_id);
}

typedef void (*free_func)(void *data);
typedef void (*convert_func)(void *data, void *goal, int id);

static void
_fill_array(Numbered_Array *array, size_t type_size, void *data, unsigned int n_data, size_t data_type_size, free_func ff, convert_func cf)
{
    int diff = n_data - array->length;

    /*Array is really array, data content is array of pointers */
    for (int i = 0; i < array->length ; ++i)
      ff(array->data+i*type_size); \

    array->data = realloc(array->data, n_data * type_size);

    if (diff > 0)
      memset(array->data + array->length * type_size, 0, type_size * diff);

    array->length = n_data;

    for (int i = 0; i < array->length ; i ++)
      {
         void** from = data + i * data_type_size;
         void* to = array->data + i * type_size;
         cf(*from, to, i); \
      }
}

#define CONVERT_ARRAY(array, array_type, data, n_data, data_type, ff, cf) \
    _fill_array(&array, sizeof(array_type), data, n_data, sizeof(data_type), ff, cf)


static void
_data_convert(Sp_Client_Context *ctx, Spawny__Server__Data *data)
{
    printf("Sessions\n");
    CONVERT_ARRAY(ctx->sessions, Session,
                   data->sessions, data->n_sessions, Spawny__Server__Session*,
                   _session_free, _session_convert);
    printf("Private Sessions\n");
    CONVERT_ARRAY(ctx->private_session, Private_Session,
                   data->sessions, data->n_sessions, Spawny__Server__Session*,
                   _private_session_free, _private_session_convert);
    printf("Users\n");
    CONVERT_ARRAY(ctx->users, User,
                   data->users, data->n_users, Spawny__Server__User*,
                   _user_free, _user_convert);
    printf("Templates\n");
    CONVERT_ARRAY(ctx->templates, Template,
                   data->templates, data->n_templates, Spawny__Server__SessionTemplate*,
                   _template_free, _template_convert);

    printf("Private Template\n");
    CONVERT_ARRAY(ctx->private_templates, Private_Template,
                   data->templates, data->n_templates, Spawny__Server__SessionTemplate*,
                   _private_template_free, _private_template_convert);

}
