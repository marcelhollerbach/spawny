#include "main.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <ini.h>

#define EXT ".desktop"

typedef struct {
    char *name;
    char *exec;
    char *icon;
} X11_Session;

static void
_fire_up(void *data) {
    execl("/usr/bin/startx", "startx", data, NULL);
}

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
_parse_file(const char *file) {
    static X11_Session session;

    if (!ini_parse(file, _parse_file_handler, &session)) {
        template_register(session.name, session.icon, _fire_up, session.exec);
    }
}

void
_parse_dir(const char *directory_path) {
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
            _parse_file(path);
        }
    }
}

void
x11_template_init(void) {
    _parse_dir("/usr/share/xsessions/");
    _parse_dir("/usr/local/share/xsessions/");
}
