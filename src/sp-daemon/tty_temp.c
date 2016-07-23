#include "main.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libgen.h>

static void
_tty_fire_up(void *data) {
    uid_t uid;
    struct passwd *pwd;

    uid = getuid();
    pwd = getpwuid(uid);

    execl(pwd->pw_shell, basename(pwd->pw_shell), NULL);
    perror("Execl failed");
}

void
tty_template_init(void) {
    const char *temp;
    if (!(temp = template_register("tty", NULL, _tty_fire_up, NULL)))
      printf("Failed to register template\n");
}
