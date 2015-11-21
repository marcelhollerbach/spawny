#include "main.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define WRITE_COM_PATH "/tmp/spawny.write"
#define READ_COM_PATH "/tmp/spawny.read"

#define MAX_MSG_SIZE 4096

static int read_file = 0;
static int write_file = 0;

static Spawny__Server__Data system_data = SPAWNY__SERVER__DATA__INIT;

static void
_send_message(Spawny__Server__MessageType type, Spawny__Server__LoginFeedback *fb, Spawny__Server__Data *data) {
    Spawny__Server__Message msg = SPAWNY__SERVER__MESSAGE__INIT;
    int len;
    void *buf;

    msg.type = type;
    msg.data = data;
    msg.feedback = fb;

    len = spawny__server__message__get_packed_size(&msg);

    buf = malloc(len);

    spawny__server__message__pack(&msg, buf);

    if (write(write_file, buf, len) < 1) {
        printf("Message write failed\n");
    }

    free(buf);
}

static void
_session_job(void *data) {
    if (!template_run(data))
      printf("Failed to run template, template not found");
}

void
server_spawnservice_feedback(int success, char *message) {
    Spawny__Server__LoginFeedback msg = SPAWNY__SERVER__LOGIN__FEEDBACK__INIT;

    if (!write_file) return;

    msg.login_success = success;
    msg.msg = message;

    _send_message(SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK, &msg, NULL);
}

static void
_greeter_msg_process(Spawny__Greeter__Message *msg) {
    switch(msg->type){
        case SPAWNY__GREETER__MESSAGE__TYPE__HELLO:
            write_file = open(WRITE_COM_PATH, O_WRONLY);
            _send_message(SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE, NULL, &system_data);
            printf("Greeter said hello, wrote system_data\n");
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__SESSION_ACTIVATION:
            session_activate(msg->session);
            manager_stop();
            printf("Greeter session activation\n");
        break;
        case SPAWNY__GREETER__MESSAGE__TYPE__LOGIN_TRY:
            if (!spawnservice_spawn(_session_job, "entrance", msg->login->user, msg->login->password, msg->login->template_id)) {
                server_spawnservice_feedback(0, "spawn failed.");
            }
            printf("Greeter login try\n");
        break;
        default:
        break;
    }
}

static void
_greeter_data(void *data, int fd) {
    uint8_t buf[MAX_MSG_SIZE];
    Spawny__Greeter__Message *message = NULL;
    int len = 0;

    len = read(fd, buf, sizeof(buf));

    if (len == 0) {
        printf("Greeter dissapeared!\n");
        //the fd is at the end
        manager_unregister_fd(fd);
        return;
    }

    message = spawny__greeter__message__unpack(NULL, len, buf);

    _greeter_msg_process(message);

    spawny__greeter__message__free_unpacked(message, NULL);
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

    if (fptr == NULL)
      {
         printf("Failed to read passwd\n");
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

    for (int i = 0;i < number - offset;i ++){
        uid_t uid;
        struct passwd* user;
        sessions[i - offset] = calloc(1, sizeof(Spawny__Server__Session));

        spawny__server__session__init(sessions[i]);

        if (!session_details(sessions_raw[i], &uid, &sessions[i]->icon, &sessions[i]->name)) {
            free(sessions[i]);
            printf("%s %d failed to fetch details\n", sessions_raw[i], i);
            offset ++;
            sessions = realloc(sessions, number * sizeof(Spawny__Server__Session));
            continue;
        }

        user = getpwuid(uid);

        sessions[i - offset]->id = sessions_raw[i];
        sessions[i - offset]->user = user->pw_name;
    }

    system_data.sessions = sessions;
    system_data.n_sessions = number - offset;

    template_get(&templates_raw, &number);

    templates = calloc(number, sizeof(Spawny__Server__SessionTemplate*));

    for(int i = 0; i< number; i++){
        templates[i] = calloc(1, sizeof(Spawny__Server__SessionTemplate));

        spawny__server__session__template__init(templates[i]);
        templates[i]->id = templates_raw[i];
        template_details_get(templates_raw[i], (const char**)&templates[i]->name, (const char**)&templates[i]->icon);
    }

    system_data.templates = templates;
    system_data.n_templates = 1;
}

int
server_init(void) {
    struct passwd *pw;

    pw = getpwnam(config->greeter.start_user);

    if (mkfifo(WRITE_COM_PATH, 0) < 0 ||
        mkfifo(READ_COM_PATH, 0) < 0) {
         //startup failed
        unlink(WRITE_COM_PATH);
        unlink(READ_COM_PATH);

        if (mkfifo(WRITE_COM_PATH, 0) < 0 ||
            mkfifo(READ_COM_PATH, 0) < 0) {
            printf("Fifos error failed to repair\n");
            return 0;
        }
      }

    chmod(WRITE_COM_PATH, S_IRUSR | S_IWUSR);
    chmod(READ_COM_PATH, S_IRUSR | S_IWUSR);

    chown(WRITE_COM_PATH, pw->pw_gid, 0);
    chown(READ_COM_PATH, pw->pw_gid, 0);

    //start reading
    read_file = open(READ_COM_PATH, O_RDONLY | O_NONBLOCK);

    //register this fd for reading
    manager_register_fd(read_file, _greeter_data, NULL);

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
    IT_FREE(system_data.n_users, system_data.users);
    IT_FREE(system_data.n_sessions, system_data.sessions);
    IT_FREE(system_data.n_templates, system_data.templates);

    if (write_file) {
      printf("Shutdown greeter\n");
      _send_message(SPAWNY__SERVER__MESSAGE__TYPE__LEAVE, NULL, NULL);
    }

    if (read_file)
      close(read_file);
    if (write_file)
      close(write_file);

    unlink(WRITE_COM_PATH);
    unlink(READ_COM_PATH);
}