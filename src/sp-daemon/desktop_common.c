#include "main.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <ini.h>
typedef struct {
    char *name;
    char *exec;
    char *icon;
} X11_Session;

#define EXT ".desktop"

static int
_parse_file_handler(void* user, const char* section,
              const char* name, const char* value) {
    X11_Session *session = user;
    if (!strcmp(name, "Name")) {
        session->name = strdup(value);
    } else if (!strcmp(name, "Exec")) {
        session->exec = strdup(value);
    } else if (!strcmp(name, "Icon")) {
        session->icon = strdup(value);
    } else {
        //printf("Parsing does not understand [%s] %s=%s\n", section, name, value);
        return 1;
    }
    return 1;
}

void
_parse_file(const char *file, Template_Fire_Up fire_up) {
    X11_Session session = { NULL };

    if (!parse_ini_verbose(file, _parse_file_handler, &session)) {
        printf("Failed to parse %s\n", file);
        return;
    }

    if (!session.name) {
        printf("Session name not found %s\n", file);
        goto not_register;
    }

    if (!session.exec) {
        printf("Session exec not found %s\n", session.name);
        goto not_register;
    }

    if (!session.icon) {
        printf("Session icon not set for %s.\n", session.name);
    }

    template_register(session.name, session.icon, fire_up, session.exec);

    return;
not_register:
    if (session.name) free(session.name);
    if (session.exec) free(session.exec);
    if (session.icon) free(session.icon);
}

void
parse_dir(const char *directory_path, Template_Fire_Up fire_up) {
    DIR *directory;
    struct dirent *dir;

    directory = opendir(directory_path);

    if (!directory) return;

    while((dir = readdir(directory))) {
        char path[PATH_MAX];
        int len;

        len = strlen(dir->d_name);

        if (!strcmp(dir->d_name + len - sizeof(EXT) + 1, EXT)) {
            snprintf(path, sizeof(path), "%s/%s", directory_path, dir->d_name);
            _parse_file(path, fire_up);
        }
    }
}

/**
 * 0 on fail
 * 1 on success
 */
int
parse_ini_verbose(const char* filename, ini_handler handler, void* user) {
    int ret;

    ret = ini_parse(filename, handler, user);

    if (!ret) return 1; /* yey no error */

    if (ret == -1) {
        fprintf(stderr, "Failed to open %s\n", filename);
    } else if (ret == -2) {
        fprintf(stderr, "Failed to allocate memory on parsing %s.\n", filename);
    } else {
        fprintf(stderr, "Failed to parse %s. Error on line %d\n", filename, ret);
    }
    return 0;
}
