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
    }
    return 1;
}

void
_parse_file(const char *file, Template_Fire_Up fire_up) {
    static X11_Session session;

    if (!ini_parse(file, _parse_file_handler, &session)) {
        template_register(session.name, session.icon, fire_up, session.exec);
    }
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