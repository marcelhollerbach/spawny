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

#define MAX_MSG_SIZE 4096

static void _data_convert(Sp_Client_Context *ctx, Spawny__Server__Data *data);

#define API_ENTRY(purpose_value) \
    if (!ctx) { \
        ERR("Library not initializied\n"); \
        return 0; \
    }  \
    if (ctx->purpose != purpose_value) { \
        ERR("Library in wrong purpose-state\n"); \
        return 0; \
    }

#define API_CALLBACK_CHECK() \
    if (!ctx) { \
        ERR("Library not initializied\n"); \
        return 0; \
    }

struct _Sp_Client_Context{
   bool debug;
   int server_sock;
   Spawny__Server__Data server_data;
   Numbered_Array sessions;
   Numbered_Array private_sessions;
   Numbered_Array templates;
   Numbered_Array private_templates;
   Numbered_Array users;
};

#define WRITE(c, m) if (!_write_msg(c, m)) { ERR("Write failed.") return false; }

//Helper functions

static bool
_write_msg(Sp_Client_Context *ctx, Spawny__Greeter__Message *msg) {
    uint8_t *buf;
    int len;

    len = spawny__greeter__message__get_packed_size(msg);
    buf = malloc(len);

    len = spawny__greeter__message__pack(msg, buf);

    if (write(ctx->server_sock, buf, len) != len) {
        perror("Writing message failed");
        return false;
    }

    free(buf);

    return true;
}

static bool
_interface_check(Sp_Client_Interface *interface) {
    if (!interface) return 0;
    if (!interface->feedback_cb) {
        ERR("feedback_cb cannot be NULL\n");
        return false;
    }
    if (!interface->data_cb) {
        ERR("data_cb cannot be NULL\n");
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


PUBLIC_API bool
sp_client_session_activate(Sp_Client_Context *ctx, int session) {
    API_CALLBACK_CHECK()

    if (session < 0 || session >= ctx->private_sessions.length)
      {
         ERR("The session(%d) is invalid!\n", session);
         return false;
      }

    Private_Session ps = ARRAY_ACCESS(&ctx->private_sessions, Private_Session, session);

    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION;
    msg.session = (char*) ps.session_id;

    WRITE(ctx, &msg)

    return true;
}

PUBLIC_API bool
sp_client_login(Sp_Client_Context *ctx, const char *usr, const char *pw, int template) {
    API_CALLBACK_CHECK()

    if (template < 0 || template >= ctx->private_templates.length)
      {
         ERR("The template(%d) is invalid!\n", template);
         return false;
      }

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


PUBLIC_API void
_parse_args(Sp_Client_Context *ctx, int argc, char *argv[])
{
   for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")) {
            ctx->debug = true;
        }
   }
}

PUBLIC_API Sp_Client_Context*
sp_client_init(int argc, char *argv[]) {
    Sp_Client_Context *ctx;

    ctx = calloc(1, sizeof(Sp_Client_Context));

    ctx->debug = false;
    if (argc > 0 && argv)
      _parse_args(ctx, argc, argv);

    ctx->server_sock = sp_service_connect(ctx->debug);

    if (ctx->server_sock < 0) {
        //service_connect() is writing error messages
        goto end;
    }

    if (_sp_client_hello(ctx))
      return ctx;

end:
    if (ctx->server_sock > 0) close(ctx->server_sock);
    free(ctx);
    return NULL;
}

PUBLIC_API bool
sp_client_greeter_start(int argc, char *argv[]) {
    Spawny__Server__Message *msg = NULL;
    uint8_t buf[MAX_MSG_SIZE];
    Sp_Client_Context ctx;
    int len = 0;

    memset(&ctx, 0, sizeof(Sp_Client_Context));

    if (argc > 0 && argv)
     _parse_args(&ctx, argc, argv);

    ctx.server_sock = sp_service_connect(ctx.debug);

    _sp_client_start_greeter(&ctx);

    len = read(ctx.server_sock, buf, sizeof(buf));

    if (len == -1) {
        perror("Reading the socket connection failed");
        return false;
    }

    msg = spawny__server__message__unpack(NULL, len, buf);

    if (!msg) {
        ERR("Invalid data object");
        return false;
    }

    if (msg->type != SPAWNY__SERVER__MESSAGE__TYPE__LEAVE) {
        ERR("Got unexpected message");
        return false;
    }

    spawny__server__message__free_unpacked(msg, NULL);

    return true;
}

PUBLIC_API int
sp_client_fd_get(Sp_Client_Context *ctx)
{
   return ctx->server_sock;
}

PUBLIC_API bool
sp_client_free(Sp_Client_Context *ctx) {
    close(ctx->server_sock);
    free(ctx);

    return true;
}

//read data packages
PUBLIC_API Sp_Client_Read_Result
sp_client_read(Sp_Client_Context *ctx, Sp_Client_Interface *interface) {
    API_CALLBACK_CHECK()
    Spawny__Server__Message *msg = NULL;
    int len = 0;
    uint8_t buf[MAX_MSG_SIZE];

    if (!_interface_check(interface)) return SP_CLIENT_READ_RESULT_FAILURE;

    len = read(ctx->server_sock, buf, sizeof(buf));

    if (len < 1) {
        perror("Reading the socket connection failed");
        return SP_CLIENT_READ_RESULT_FAILURE;
    }

    msg = spawny__server__message__unpack(NULL, len, buf);

    if (!msg) {
        ERR("Invalid data object\n");
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
            printf("Feedback!\n");
            interface->feedback_cb(msg->feedback->success, msg->feedback->msg);
        break;
        default:
            printf("Unknown messsage type\n");
        break;
    }
    return SP_CLIENT_READ_RESULT_SUCCESS;
}

PUBLIC_API void
sp_client_data_get(Sp_Client_Context *ctx, Numbered_Array *sessions, Numbered_Array *templates, Numbered_Array *users)
{
    if (sessions) memcpy(sessions, &ctx->sessions, sizeof(Numbered_Array));
    if (templates) memcpy(templates, &ctx->templates, sizeof(Numbered_Array));
    if (users) memcpy(users, &ctx->users, sizeof(Numbered_Array));
}

static int
_find_simular_session(Sp_Client_Context *ctx, const char *prefereed_session)
{
   for (int i = 0; i < ctx->templates.length; ++i)
   {
       Template *temp = &TEMPLATE_ARRAY(&ctx->templates, i);
       if (!strcmp(prefereed_session, temp->name))
         return i;
   }
   return 0;
}


//=============================================================
// Code for converting protoobjects to normal objects follows
//=============================================================

typedef void (*free_func)(void *data);
typedef void (*convert_func)(Sp_Client_Context *ctx, void *data, void *goal, int id);

#define FREE_S(n) if (n) free(n); \
                  n = NULL;

#define STRDUP_S(n, v) if (n) v = strdup(n);

#define FREE_CONVERT(name, type, proto_type, conv, free) \
static void \
name ##s_convert(Sp_Client_Context *ctx, void *data, void *goal, int id) { \
    proto_type proto = data; \
    type name = goal; \
    conv \
} \
static void \
name ##s_free(void *data) { \
    type name = data; \
    free \
}

FREE_CONVERT(template, Template*, Spawny__Server__SessionTemplate*, {
        template->id = id;
        STRDUP_S(proto->icon, template->icon);
        STRDUP_S(proto->name, template->name);
    },{
        FREE_S(template->name);
        FREE_S(template->icon);
    })

FREE_CONVERT(session, Session*, Spawny__Server__Session*, {
        session->id = id;
        STRDUP_S(proto->icon, session->icon);
        STRDUP_S(proto->name, session->name);
        STRDUP_S(proto->user, session->user);
    },{
        FREE_S(session->icon);
        FREE_S(session->name);
        FREE_S(session->user);
    })

FREE_CONVERT(user, User*, Spawny__Server__User*, {
        user->id = id;
        STRDUP_S(proto->icon, user->icon);
        STRDUP_S(proto->name, user->name);
        user->prefered_session = _find_simular_session(ctx, proto->prefered_session);
    },{
        FREE_S(user->icon);
        FREE_S(user->name);
    })

FREE_CONVERT(private_session, Private_Session*, Spawny__Server__Session*,{
        private_session->id = id;
        STRDUP_S(proto->id, private_session->session_id);
    },{
        FREE_S(private_session->session_id);
    })

FREE_CONVERT(private_template, Private_Template*, Spawny__Server__SessionTemplate*, {
        private_template->id = id;
        STRDUP_S(proto->id, private_template->template_id);
    },{
        FREE_S(private_template->template_id);
    })

#undef FREE_CONVERT
#undef STRDUP_S
#undef FREE_S

static void
_fill_array(Sp_Client_Context *ctx, Numbered_Array *array, size_t type_size, void *data, unsigned int n_data, size_t data_type_size, free_func ff, convert_func cf)
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
         cf(ctx, *from, to, i); \
      }
}

static void
_data_convert(Sp_Client_Context *ctx, Spawny__Server__Data *data)
{

#define CONVERT_ARRAY_P(name, data_name, array_type, data_type) \
    _fill_array(ctx, &ctx->name, sizeof(array_type), data->data_name, data->n_ ##data_name, sizeof(data_type), name ##_free, name ##_convert)
#define CONVERT_ARRAY(name, array_type, data_type) CONVERT_ARRAY_P(name, name, array_type, data_type)

    CONVERT_ARRAY(templates, Template, Spawny__Server__SessionTemplate*);
    CONVERT_ARRAY(sessions, Session, Spawny__Server__Session*);
    CONVERT_ARRAY(users, User, Spawny__Server__User*);
    CONVERT_ARRAY_P(private_sessions, sessions, Private_Session, Spawny__Server__Session*);
    CONVERT_ARRAY_P(private_templates, templates, Private_Template, Spawny__Server__SessionTemplate*);

#undef CONVERT_ARRAY
#undef CONVERT_ARRAY_P
}
