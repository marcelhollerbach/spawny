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
    char executor[] = "startx";
    char command[strlen(executor) + strlen(data) + 2];
    uid = getuid();
    pwd = getpwuid(uid);

    snprintf(command, sizeof(command), "%s %s", executor, data);

    execl(pwd->pw_shell, basename(pwd->pw_shell), "--login", "-i", "-c", command, NULL);
    perror("Execl failed");
}

void
x11_template_init(void) {
    parse_dir("/usr/share/xsessions/", "x11", _fire_up);
    parse_dir("/usr/local/share/xsessions/", "x11", _fire_up);
}
