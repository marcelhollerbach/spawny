#include "main.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libgen.h>

static void
_fire_up(void *data) {
    uid_t uid;
    struct passwd *pwd;

    uid = getuid();
    pwd = getpwuid(uid);
    execl(pwd->pw_shell, basename(pwd->pw_shell), "-c", data);
}

void
wl_template_init(void) {
    parse_dir("/usr/share/wayland-sessions/", _fire_up);
    parse_dir("/usr/local/share/wayland-sessions/", _fire_up);
}
