#include "main.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <ini.h>

#define EXT ".desktop"


static void
_fire_up(void *data) {
    execl("/usr/bin/startx", "startx", data, NULL);
}

void
x11_template_init(void) {
    parse_dir("/usr/share/xsessions/", _fire_up);
    parse_dir("/usr/local/share/xsessions/", _fire_up);
}
