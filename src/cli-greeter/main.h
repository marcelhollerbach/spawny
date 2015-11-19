#ifndef MAIN_H
#define MAIN_H

#include "../protocol/protocol.h"

typedef void (*Data_Cb)(Spawny__Server__Data *data);
typedef void (*Login_Feedback_Cb)(int succes, char *msg);


int client_init(Data_Cb _data_cb, Login_Feedback_Cb _login_cb);
int client_login(char *usr, char *pw, char *template);
int client_run(void);
int client_shutdown(void);

#endif
