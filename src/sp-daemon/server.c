#define _GNU_SOURCE

#include "main.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <systemd/sd-daemon.h>

#define MAX_MSG_SIZE 4096

typedef struct {
    int fd;
    struct ucred client_info;
} Client;

static int server_sock;
static void _load_data(void);

static Spawny__Server__Data system_data = SPAWNY__SERVER__DATA__INIT;

static void
_send_message(Spawny__Server__MessageType type, Spawny__Server__RequestFeedback *fb, Spawny__Server__Data *data, int fd) {
    Spawny__Server__Message msg = SPAWNY__SERVER__MESSAGE__INIT;
    int len = 0;
    void *buf;

    msg.type = type;
    msg.data = data;
    msg.feedback = fb;

    len = spawny__server__message__get_packed_size(&msg);

    buf = malloc(len);

    spawny__server__message__pack(&msg, buf);

    if (write(fd, buf, len) != len)
      perror("greeter message failed");

    free(buf);
}

static void
_session_job(void *data) {
    if (!template_run(data))
      ERR("Failed to run template, template not found");
}

void
server_spawnservice_feedback(int success, const char *message, int fd) {
    Spawny__Server__RequestFeedback msg = SPAWNY__SERVER__REQUEST__FEEDBACK__INIT;

    msg.success = success;
    msg.msg = (char*) message;

    _send_message(SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK, &msg, NULL, fd);
}

static void
client_free(Client *client) {
    manager_unregister_fd(client->fd);
    close(client->fd);
}

static void
_session_done(void *data, Spawn_Service_End end) {
    Client *client = data;
    if (end.success == SPAWN_SERVICE_ERROR) {
        server_spawnservice_feedback(0, end.message, client->fd);
    } else {
        greeter_lockout(seat_get(client->client_info.pid));
        server_spawnservice_feedback(1, "You are logged in!", client->fd);
        INF("User Session alive.");
        client_free(data);
    }

}

static void
_client_data(Fd_Data *data, int fd) {
    Client *client;
    Spawny__Greeter__Message *msg = NULL;
    uint8_t buf[PATH_MAX];
    int len = 0;
    const char *seat;

    client = data->data;
    len = read(fd, buf, sizeof(buf));

    if (!len)
      {
         manager_unregister_fd(fd);
         return;
      }

    msg = spawny__greeter__message__unpack(NULL, len, buf);

    if (!msg)
      {
         ERR("Failed to receive message from %d.", fd);
         manager_unregister_fd(fd);
         return;
      }

    switch(msg->type){
        case SPAWNY__GREETER__MESSAGE__TYPE__HELLO:
            INF("Greeter said ehllo, wrote system_data");
            //load the newesr available data
            _load_data();
            _send_message(SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE, NULL, &system_data, fd);
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION:
            INF("Greeter session activation %s", msg->session);
            session_activate(msg->session);
            greeter_lockout(seat_get(client->client_info.pid));
            client_free(client);
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY:
            INF("Greeter login try");
            if (!spawnservice_spawn(_session_done, client, _session_job, msg->login->template_id, PAM_SERVICE, msg->login->user, msg->login->password)) {
                server_spawnservice_feedback(0, "spawn failed.", fd);
            }
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__GREETER_START:
            INF("Greeter start");
            seat = seat_get(client->client_info.pid);
            if (!seat) seat = "seat0";
            greeter_activate(seat);
            client_free(client);
            client = NULL;
        break;
        default:
        break;
    }
    spawny__greeter__message__free_unpacked(msg, NULL);
}


static Client*
client_connect(int fd)
{
    Client *client;
    socklen_t len;

    client = calloc(1, sizeof(Client));

    len = sizeof(client->client_info);

    if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED, &client->client_info, &len) != 0 || len != sizeof(client->client_info)) {
        perror("Failed to fetch client info");
        free(client);
        return NULL;
    }

    client->fd = fd;

    return client;
}

static void
_accept_ready(Fd_Data *data, int fd)
{
    socklen_t size;
    int client;
    Client *inst;
    client = accept(fd,(struct sockaddr*) NULL, &size);

    inst = client_connect(client);

    if (!inst) return;

    manager_register_fd(client, _client_data, inst);
}

static void
_load_sessions(void)
{
    unsigned int offset = 0, number = 0;
    Spawny__Server__Session **sessions;
    char **sessions_raw;

    session_enumerate(&sessions_raw, &number);

    sessions = calloc(number, sizeof(Spawny__Server__Session*));

    for (int i = 0;i < number;i ++){
        uid_t uid;
        struct passwd* user;
        sessions[i - offset] = calloc(1, sizeof(Spawny__Server__Session));
        spawny__server__session__init(sessions[i - offset]);

        if (!session_details(sessions_raw[i], &uid, &sessions[i - offset]->icon, &sessions[i - offset]->name, NULL)) {
            free(sessions[i - offset]);
            sessions[i - offset] = NULL;
            ERR("%s %d failed to fetch details", sessions_raw[i], i);
            offset ++;
            sessions = realloc(sessions, (number - offset) * sizeof(Spawny__Server__Session));
            continue;
        }

        user = getpwuid(uid);

        sessions[i - offset]->id = sessions_raw[i];
        sessions[i - offset]->user = user->pw_name;
    }

    system_data.n_sessions = number - offset;
    system_data.sessions = sessions;
}

static void
_load_users(void)
{
    Spawny__Server__User **user;
    char **usernames;
    unsigned int number;

    number = user_db_users_iterate(&usernames);

    user = calloc(number, sizeof(Spawny__Server__User*));

    for(int i = 0; i < number; i++) {
        struct passwd *pass;
        const char *field;

        user[i] = calloc(1, sizeof(Spawny__Server__User));

        spawny__server__user__init(user[i]);

        pass = getpwnam(usernames[i]);
        field = user_db_field_get(usernames[i], "icon");

        if (field)
          user[i]->icon = strdup(field);
        else
          user[i]->icon = NULL;

        user[i]->uid = pass->pw_uid;
        user[i]->name = strdup(pass->pw_name);

        field = user_db_field_get(usernames[i], "prefered-session");

        if (field)
          user[i]->prefered_session = strdup(field);
        else
          user[i]->prefered_session = NULL;

        free(usernames[i]);
    }
    free(usernames);

    system_data.users = user;
    system_data.n_users = number;

}

static void
_load_templates(void)
{
    char **templates_raw;

    Spawny__Server__SessionTemplate **templates;
    unsigned int number = 0;

    template_get(&templates_raw, &number);

    templates = calloc(number, sizeof(Spawny__Server__SessionTemplate*));

    for(int i = 0; i< number; i++){
        templates[i] = calloc(1, sizeof(Spawny__Server__SessionTemplate));

        spawny__server__session__template__init(templates[i]);
        templates[i]->id = templates_raw[i];
        template_details_get(templates_raw[i], (const char**)&templates[i]->name, (const char**)&templates[i]->icon);
    }

    free(templates_raw);

    system_data.templates = templates;
    system_data.n_templates = number;

}

static void
_load_data(void) {
    _load_templates();
    _load_users();
    _load_sessions();
}

static int
_socket_setup(void)
{
    int n;
    struct sockaddr_un address;

    n = sd_listen_fds(0);
    if (n > 1) {
        ERR("Error, systemd passed to many fd´s!");
        return 0;
    } else if (n == 1) {
        server_sock = SD_LISTEN_FDS_START + 0;
    } else {
        const char *path;

        path = sp_service_path_get(debug);

        server_sock = sp_service_socket_create();

        sp_service_address_setup(debug, &address);

        //try to remove the path before binding it
        unlink(path);

        if (bind(server_sock, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
            perror("Failed to establish server connection");
            return 0;
        }

        if (listen(server_sock, 3) != 0) {
            perror("Failed to listen on the server");
            return 0;
        }

        //set permissions on the socket
        if (chmod(path, 0777) == -1) {
          perror("Failed to chmod");
        }
    }
    return 1;
}

int
server_init(void) {

    //setup server
    if (!_socket_setup())
      return 0;

    user_db_init();

    //listen on the socket
    manager_register_fd(server_sock, _accept_ready, NULL);

    return 1;
}

#define IT_FREE(num, it) \
    if (num > 0) { \
        for (int i = 0; i < num; i++) { \
            free(it[i]); \
        } \
        free(it); \
    }

void
server_shutdown(void) {
    const char *path;

    path = sp_service_path_get(debug);

    IT_FREE(system_data.n_users, system_data.users);
    IT_FREE(system_data.n_sessions, system_data.sessions);
    IT_FREE(system_data.n_templates, system_data.templates);
    user_db_shutdown();
    close(server_sock);
    unlink(path);
}
