#include "main.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libgen.h>

static void
_tty_fire_up(void) {
    uid_t uid;
    struct passwd *pwd;

    uid = getuid();
    pwd = getpwuid(uid);

    execl(pwd->pw_shell, basename(pwd->pw_shell), NULL);
    printf("If you can read this its bad\n");
}

void
tty_template_init(void) {
    const char *temp;
    if (!(temp = template_register("tty", NULL, _tty_fire_up)))
      printf("Failed to register template\n");
    printf("Registered %s\n", temp);
}
