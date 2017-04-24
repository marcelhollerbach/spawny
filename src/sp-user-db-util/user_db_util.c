#include "user_db.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_VALUE 1048
static char *username;

static void
_print_help(void)
{
   printf("The following options are supported\n");
   printf(" --icon name\n");
   printf(" --prefered-session session\n");
   printf(" --edit key,val\n");
   printf(" --del key\n");
   printf("The current keys are used by the daemon: prefered-session");
}

static bool
_field_edit(const char *arg) {
    char buf[MAX_VALUE];
    char *value, *name;

    strcpy(buf, arg);

    name = strtok(buf, ",");
    value  = strtok(NULL, ",");

    if (!user_db_field_set(username, name, value)) return false;

    printf("Adding field %s with value %s\n", name, value);

    return true;
}

static bool
_field_del(const char *arg) {

    if (!user_db_field_del(username, arg)) return false;

    printf("Deleting field %s\n", arg);
    return true;
}


int main(int argc, char const *argv[])
{
    if (!user_db_init())
      return EXIT_FAILURE;

    for(int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--user")) {
            username = strdup(argv[i + 1]);
            break;
        }
    }

    if (!username) {
        struct passwd *wd;
        uid_t uid;

        uid = getuid();
        wd = getpwuid(uid);
        username = strdup(wd->pw_name);
    }

    if (!user_db_user_exists(username)) {
        printf("User %s not found!\n", username);
        return EXIT_FAILURE;
    }

    for(int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--edit")) {
            i++;
            if (!_field_edit(argv[i])) return EXIT_FAILURE;
        } else if (!strcmp(argv[i], "--icon")) {
            i++;
            /**FIXME maybe we should import that thing**/
            if (!user_db_field_set(username, "icon", argv[i])) return EXIT_FAILURE;
        } else if (!strcmp(argv[i], "--prefered-session")) {
            i++;
            if (!user_db_field_set(username, "prefered-session", argv[i])) return EXIT_FAILURE;
        } else if (!strcmp(argv[i], "--del")) {
            i++;
            if (!_field_del(argv[i])) return EXIT_FAILURE;
        } else if (!strcmp(argv[i], "--help")) {
            _print_help();
            return EXIT_FAILURE;
        } else {
            printf("unrecognized option %s\n", argv[i]);
            _print_help();
            return EXIT_FAILURE;
        }
    }


    user_db_shutdown();
    free(username);

    return EXIT_FAILURE;
}
