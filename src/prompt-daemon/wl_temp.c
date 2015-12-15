#include "main.h"

#include <unistd.h>
#include <libgen.h>

static void
_fire_up(void *data) {
    execl(data, basename(data), NULL);
}

void
wl_template_init(void) {
    parse_dir("/usr/share/wayland-sessions/", _fire_up);
    parse_dir("/usr/local/share/wayland-sessions/", _fire_up);
}
