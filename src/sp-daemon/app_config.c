#include "main.h"
#include <string.h>
#include "ini.h"

Config *config;

static int
_config_parse(void* user, const char* section,
              const char* name, const char* value) {
    if (!strcmp(section, "greeter")) {
        if (!strcmp(name, "user"))
          config->greeter.start_user = value;
        if (!strcmp(name, "cmd"))
          config->greeter.cmd = value;
    }

    return 1;
}

int
config_init(void) {
    config = calloc(1, sizeof(Config));

    config->greeter.start_user = "spawny";
    config->greeter.cmd = NULL;

    ini_parse(PACKAGE_CONFIG, _config_parse, NULL);

    return 1;
}

int
config_shutdown(void) {
    return 1;
}
