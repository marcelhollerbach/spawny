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
#include <errno.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 4096

static int server_sock;

static Spawny__Server__Data system_data = SPAWNY__SERVER__DATA__INIT;

static void
_send_message(Spawny__Server__MessageType type, Spawny__Server__LoginFeedback *fb, Spawny__Server__Data *data, int fd) {
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
      printf("Failed to run template, template not found");
}

void
server_spawnservice_feedback(int success, const char *message, int fd) {
    Spawny__Server__LoginFeedback msg = SPAWNY__SERVER__LOGIN__FEEDBACK__INIT;

    msg.login_success = success;
    msg.msg = (char*) message;

    _send_message(SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK, &msg, NULL, fd);
}

static void
_session_done(void *data, Spawn_Service_End end) {
    int fd = (int)data;
    if (end.success == SPAWN_SERVICE_ERROR) {
        server_spawnservice_feedback(0, end.message, fd);
    } else {
        server_spawnservice_feedback(1, "You are logged in!", fd);
        printf("User Session alive.\n");
        manager_unregister_fd(fd);
        close(fd);
    }

}

static void
_client_data(Fd_Data *data, int fd) {
    Spawny__Greeter__Message *msg = NULL;
    uint8_t buf[PATH_MAX];
    int len = 0;

    len = read(fd, buf, sizeof(buf));

    if (!len)
      {
         manager_unregister_fd(fd);
         return;
      }

    msg = spawny__greeter__message__unpack(NULL, len, buf);

    switch(msg->type){
        case SPAWNY__GREETER__MESSAGE__TYPE__HELLO:
            _send_message(SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE, NULL, &system_data, fd);
            printf("Greeter said hello, wrote system_data\n");
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION:
            session_activate(msg->session);
            printf("Greeter session activation %s\n", msg->session);
            manager_unregister_fd(fd);
            close(fd);
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY:
            if (!spawnservice_spawn(_session_done, (void*)(intptr_t) fd, _session_job, msg->login->template_id, PAM_SERVICE, msg->login->user, msg->login->password)) {
                server_spawnservice_feedback(0, "spawn failed.", fd);
            }
            printf("Greeter login try\n");
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__GREETER_START:
            activate_greeter();
            manager_unregister_fd(fd);
            close(fd);
        break;
        default:
        break;
    }
    spawny__greeter__message__free_unpacked(msg, NULL);
}

static void
_accept_ready(Fd_Data *data, int fd)
{
   struct sockaddr clientname;
   socklen_t size;
   int client;

   client = accept(fd, &clientname, &size);

   manager_register_fd(client, _client_data, NULL);
}
static void
_list_users(char ***usernames, unsigned int *numb) {
    FILE *fptr;
    char line[PATH_MAX];
    char *username;
    char **names = NULL;

    fptr = fopen("/etc/passwd","r");

    *numb = 0;
    *usernames = NULL;

    if (fptr == NULL) {
         perror("Failed to read passwd");
         return;
    }

    while (fgets(line, sizeof(line), fptr)) {
        username = strtok(line, ":");

        (*numb) ++;

        names = realloc(names, (*numb) * sizeof(char*));
        names[(*numb) - 1] = strdup(username);
    }
    *usernames = names;
    fclose(fptr);
}

static void
_init_data(void) {
    char **usernames, **sessions_raw, **templates_raw;
    Spawny__Server__User **user;
    Spawny__Server__Session **sessions;
    Spawny__Server__SessionTemplate **templates;
    unsigned int number;
    unsigned int offset = 0;

    _list_users(&usernames,&number);

    user = calloc(number, sizeof(Spawny__Server__User*));

    for(int i = 0; i < number; i++) {
        struct passwd *pass;

        user[i] = calloc(1, sizeof(Spawny__Server__User));

        spawny__server__user__init(user[i]);

        pass = getpwnam(usernames[i]);

        free(usernames[i]);

        user[i]->icon = NULL;
        user[i]->uid = pass->pw_uid;
        user[i]->name = pass->pw_name;
    }
    free(usernames);

    system_data.users = user;
    system_data.n_users = number;

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
            printf("%s %d failed to fetch details\n", sessions_raw[i], i);
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

    template_get(&templates_raw, &number);

    templates = calloc(number, sizeof(Spawny__Server__SessionTemplate*));

    for(int i = 0; i< number; i++){
        templates[i] = calloc(1, sizeof(Spawny__Server__SessionTemplate));

        spawny__server__session__template__init(templates[i]);
        templates[i]->id = templates_raw[i];
        template_details_get(templates_raw[i], (const char**)&templates[i]->name, (const char**)&templates[i]->icon);
    }

    system_data.templates = templates;
    system_data.n_templates = number;
}

int
server_init(void) {
    const char *path;
    struct passwd *pw;
    struct sockaddr_un address;

    path = sp_service_path_get();

    //just try it ...
    unlink(path);

    pw = getpwnam(config->greeter.start_user);
    if (!pw) {
        perror("Failed to fetch configured user");
        return 0;
    }

    server_sock = sp_service_socket_create();
    sp_service_address_setup(&address);

    if (bind(server_sock, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
        perror("Failed to establish server connection");
        return 0;
    }

    if (listen(server_sock, 5) != 0) {
        perror("Failed to listen on the server");
    }

    if (chmod(path, 0777) == -1) {
      perror("Failed to chmod");
    }

    manager_register_fd(server_sock, _accept_ready, NULL);

    //initializise data
    _init_data();

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

    path = sp_service_path_get();

    IT_FREE(system_data.n_users, system_data.users);
    IT_FREE(system_data.n_sessions, system_data.sessions);
    IT_FREE(system_data.n_templates, system_data.templates);
    close(server_sock);
    unlink(path);
}