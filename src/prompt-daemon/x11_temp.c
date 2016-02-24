#include "main.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <ini.h>
#include <pwd.h>

#define EXT ".desktop"


static void
_fire_up(void *data) {
    uid_t uid;
    struct passwd *pwd;
    char command[PATH_MAX];

    uid = getuid();
    pwd = getpwuid(uid);

    snprintf(command, sizeof(command), "startx %s", data);
    execl(pwd->pw_shell, basename(pwd->pw_shell), "-i", "-c", command);
}

void
x11_template_init(void) {
    parse_dir("/usr/share/xsessions/", _fire_up);
    parse_dir("/usr/local/share/xsessions/", _fire_up);
}
