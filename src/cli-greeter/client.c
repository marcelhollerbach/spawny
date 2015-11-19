#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define READ_COM_PATH "/tmp/spawny.write"
#define WRITE_COM_PATH "/tmp/spawny.read"

static int write_file = 0;
static int read_file = 0;
static Data_Cb data_cb;
static Login_Feedback_Cb login_feedback_cb;
static Spawny__Server__Data server_data;

static void
_write_msg(Spawny__Greeter__Message *msg) {
    uint8_t buf[PATH_MAX];
    int len;

    len = spawny__greeter__message__pack(msg, buf);

    if (write(write_file, buf, len) != len) {
        perror("Writing message failed");
    }
}

static void
_write_hello() {
    Spawny__Greeter__Message msg = SPAWNY__GREETER__MESSAGE__INIT;

    msg.type = SPAWNY__GREETER__MESSAGE__TYPE__HELLO;

    _write_msg(&msg);
}

int
client_login(char *usr, char *pw, char *template) {
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
client_init(Data_Cb _data_cb,
            Login_Feedback_Cb _login_cb) {

    data_cb = _data_cb;
    login_feedback_cb = _login_cb;

    write_file = open(WRITE_COM_PATH, O_WRONLY, 0);
    if (write_file == -1)
      perror("Write file open failed :");

    _write_hello();

    read_file = open(READ_COM_PATH, O_RDONLY, 0);
    if (read_file == -1)
      perror("Write file open failed :");

    return 1;
}

int
client_run(void) {
    Spawny__Server__Message *msg = NULL;
    int len = 0;
    uint8_t buf[PATH_MAX];

    while(1) {

        len = read(read_file, buf, sizeof(buf));
        if (len < 1) {
            perror("Read failed");
            return 0;
        }

        msg = spawny__server__message__unpack(NULL, len, buf);

        if (!msg) {
            printf("Invalid data package!!\n");
        }

        switch(msg->type) {
            case SPAWNY__SERVER__MESSAGE__TYPE__LEAVE:
                printf("Leave msg!\n");
                return 1;
            break;
            case SPAWNY__SERVER__MESSAGE__TYPE__DATA_UPDATE:
                printf("Data update!\n");
                server_data = *(msg->data);
                data_cb(&server_data);
            break;
            case SPAWNY__SERVER__MESSAGE__TYPE__LOGIN_FEEDBACK:
                printf("Login Feedback!\n");
                login_feedback_cb(msg->feedback->login_success, msg->feedback->msg);
            break;
            default:
            break;
        }

        spawny__server__message__free_unpacked(msg, NULL);
    }
    return 1;
}

int client_shutdown(void) {
    return 1;
}

