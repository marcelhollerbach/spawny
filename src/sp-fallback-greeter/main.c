#define _DEFAULT_SOURCE

#include "Sp_Client.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

static Sp_Client_Context *ctx;

int prompted = 0;

Numbered_Array templates;
Numbered_Array sessions;
Numbered_Array users;

#define PROMPT(...) printf(__VA_ARGS__);
#define MAX_INTEGER_SIZE 10+1


static int prompt_run(void);

static void
login(void) {
    char username[LOGIN_NAME_MAX], template[MAX_INTEGER_SIZE], *password;
    User *user = NULL;
    Template def;
    int temp = 0;

    PROMPT("Spawny login! (Inputs are without echo)\n");

    do {
        PROMPT("\nUsername : ");
        scanf("%s", username);
        for (int i = 0; i < users.length; ++i)
        {
            User *usr = &USER_ARRAY(&users, i);
            if (!strcmp(usr->name, username))
              {
                user = usr;
                break;
              }
        }
    } while(!user);
    def = TEMPLATE_ARRAY(&templates, user->prefered_session);

    password = getpass("Password :");
    PROMPT("");

    //consume the \n
    getchar();

    do {
        if (!user->prefered_session)
          {
            //annonce no default
            PROMPT("Session Template: (l to list templates):");
          }
        else
          {
             PROMPT("Session Template: (l to list templates, Default: %s):", def.name);
          }

        fgets(template, sizeof(template), stdin);
        printf("\n");

        //list sessions and continue
        if (template[0] == 'l') {
            //list templates
            for (int i = 0; i < templates.length; i++){
                Template template = TEMPLATE_ARRAY(&templates,i);
                printf("%d\t - \t%s\n", template.id + 1, template.name);
            }
            continue;
        }

        //fallback to default
        if (user->prefered_session > 1 && template[0] == '\n') {
            //only accept the default when the prefered session is bigger than 1
            temp = def.id;
        //or take the new id
        } else if (atoi(template) != 0) {
            temp = atoi(template);
        } else {
            printf("Unknown input %s\n", template);
            continue;
        }
        if (sp_client_login(ctx, username, password, temp - 1))
          break;
    }while(true);

    PROMPT("Waiting for feedback\n");
}

static void
listsessions(void) {
    int id;

    printf("Sessions:\n");

    for(int i = 0; i < sessions.length; i++){
        Session session = SESSION_ARRAY(&sessions, i);
        printf("    %d - %s: %s\n", session.id + 1, session.user, session.name);
    }

    PROMPT("Spawny session activation:\n");

    do {
        char session[MAX_INTEGER_SIZE];
        PROMPT("Session :");
        fgets(session, sizeof(session), stdin);
        session[strlen(session) - 1] = '\0';
        id = atoi(session);
    } while(!sp_client_session_activate(ctx, id - 1));


    PROMPT("Waiting for activation\n");
}

static int
prompt_run(void) {
    char c = '\0';

    do {
        if (c != '\0') {
          printf("\nInvalid input \"%c\"\n", c);
          printf("Iteration\n");
        }

        printf("Choose action [login(l)/listSessions(s)] default is login:");

        c = getchar();
        if (c == EOF) {
            perror("Error while read ");
            return 0;
        }
        printf("\n");

        if (c == '\n') {
            c = 'l';
        } else {
            //we need to eat the \n
            getchar();
        }

    } while (c != 'l' && c != 's');

    switch(c){
        case 'l':
            login();
        break;
        case 's':
            listsessions();
        break;
    }
    return 1;
}

static void
_data_cb(void) {

    sp_client_data_get(ctx, &sessions, &templates, &users);

    if (!prompted) {
        if (!prompt_run()) {
            printf("Failed to run prompt!!\n");
        }
    }
    prompted = 1;
}

static void
_login_cb(int success, char *msg) {
    if (!success) {
        printf("Request failed, reason %s\n", msg);
        //relogin
        prompt_run();
    } else {
        printf("Request Successfull, waiting for leave msg\n");
    }
}

static Sp_Client_Interface interface = {
    _data_cb,
    _login_cb
};

int main(int argc, char **argv) {
    Sp_Client_Read_Result res;

    //eat all the stuff which is in stdin
    printf("#########################################\n");
    printf("# Spawny Command line greeter interface #\n");
    printf("#########################################\n");
    ctx = sp_client_init(argc, argv, SP_CLIENT_LOGIN_PURPOSE_GREETER_JOB);

    if (!ctx) return EXIT_FAILURE;

    while(1) {
        res = sp_client_read(ctx, &interface);

        if (res == SP_CLIENT_READ_RESULT_EXIT || res == SP_CLIENT_READ_RESULT_FAILURE)
            break;

    }

    sp_client_free(ctx);
    ctx = NULL;

    return res == SP_CLIENT_READ_RESULT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
